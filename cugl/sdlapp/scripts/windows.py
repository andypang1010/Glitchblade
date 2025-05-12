"""
Python Script for Windows Builds

While technically Windows can run CMake, we find CMake support to be flaky. In
particular, the optional formats in sdl2image break CMake because Microsoft
MSVC compiler does not support stdatomic.h in C (sigh).  Therefore, we prefer
to use Visual Studio for Windows builds.

Author: Walker M. White
Date:   11/29/22
"""
import os, os.path
import shutil
import glob
from . import util

# The subbuild folder for the project
MAKEDIR = 'windows'

# Supported header extensions
HEADER_EXT = ['.h', '.hh', '.hpp', '.hxx', '.hm', '.inl', '.inc', '.xsd']
# Supportes source extensions
SOURCE_EXT = ['.cpp', '.c', '.cc', '.cxx', '.m', '.mm', '.def', '.odl', '.idl', '.hpj', '.bat', '.asm', '.asmx']


def build_filters(path,filetree,uuidsvc):
    """
    Creates the filters for the Visual Studio project
    
    Filters are virtual directories in Visual Studio. They must each have a
    unique UUID and be connected in a tree-like manner. This function returns
    a dictionary representing this tree.  The keys are the unique UUIDS and
    the values are three-element tuples representing each child.  These tuples
    contain the  child UUID, the child path, and an annotation. If the path is
    a leaf in the filetree, the annotation is the same as the filetree.
    Otherwise it is simply the tag 'group'.
    
    :param path: The path to the root of the filetree in POSIX form
    :type config:  ``str``
    
    :param filetree: The filetree of paths
    :type filetree:  ``dict``
    
    :param uuidsvc: The UUID generator
    :type uuidsvc:  ``UUIDService``
    
    :return: The dictionary of the filters together with the UUID of the root
    :rtype:  (``str``,``dict``)
    """
    
    uuid = uuidsvc.getWindowsUUID('GROUP:\\\\'+path)
    uuid = uuidsvc.applyPrefix('CD',uuid)
    contents = []
    result = {uuid:('',contents)}
    for item in filetree:
        if type(filetree[item]) == dict: # This is a group
            subresult = build_filters(path+'\\'+item,filetree[item],uuidsvc)
            contents.append((subresult[0],item,'group'))
            for key in subresult[1]:
                if key == subresult[0]:
                    result[key] = (item,subresult[1][key][1])
                else:
                    result[key] = subresult[1][key]
        else:
            newid = uuidsvc.getWindowsUUID('FILE:\\\\'+path+'\\'+item)
            newid = uuidsvc.applyPrefix('BA',newid)
            contents.append((newid,item,filetree[item]))
    
    return (uuid,result)


def expand_filters(path, uuid, files):
    """
    Returns the string of filters to insert into Visual Studio
    
    This string should replace __FILTER_ENTRIES__ in the filters file.
    
    :param path: The path to the root directory for the filters
    :type path:  ``str1``
    
    :param uuid: The root element of the file tree
    :type uuid:  ``str``
    
    :param files: The file tree storing both files and filters
    :type files:  ``dict``
    
    :return: The string of filters to insert into Visual Studio
    :rtype:  ``str``
    """
    result = ''
    groups = files[uuid][1]
    for entry in groups:
        if entry[2] == 'group':
            localpath = path+'\\'+entry[1]
            result += '\n    <Filter Include="%s">\n      <UniqueIdentifier>{%s}</UniqueIdentifier>\n    </Filter>' % (localpath,entry[0])
            result += expand_filters(localpath, entry[0], files)
    return result


def expand_headers(path, uuid, files, filter=True):
    """
    Returns the string of include files to insert into Visual Studio
    
    This string should replace __HEADER_ENTRIES__ in either the filters file,
    or the project file, depending upon the value of the parameter filter.
    
    :param path: The path to the root directory for the filters
    :type path:  ``str1``
    
    :param uuid: The root element of the file tree
    :type uuid:  ``str``
    
    :param files: The file tree storing both files and filters
    :type files:  ``dict``
    
    :param filter: Whether to generate the string for the filter file
    :type filter:  ``str``
    
    :return: The string of include files to insert into Visual Studio
    :rtype:  ``str``
    """
    result = ''
    groups = files[uuid][1]
    for entry in groups:
        if entry[2] in ['all', 'windows'] and not os.path.splitext(entry[1])[1] in SOURCE_EXT:
            header = path+'\\'+entry[1]
            if filter:
                result += '\n    <ClInclude Include="__ROOT_DIR__%s">\n      <Filter>Source Files%s</Filter>\n    </ClInclude>'  % (header,path)
            else:
                result += '\n    <ClInclude Include="__ROOT_DIR__%s"/>\n'  % header
        elif entry[2] == 'group':
            localpath = path+'\\'+entry[1]
            result += expand_headers(localpath, entry[0], files, filter)
    return result


def expand_sources(path, uuid, files, filter=True):
    """
    Returns the string of source files to insert into Visual Studio
    
    This string should replace __SOURCE_ENTRIES__ in either the filters file,
    or the project file, depending upon the value of the parameter filter.
    
    :param path: The path to the root directory for the filters
    :type path:  ``str1``
    
    :param uuid: The root element of the file tree
    :type uuid:  ``str``
    
    :param files: The file tree storing both files and filters
    :type files:  ``dict``
    
    :param filter: Whether to generate the string for the filter file
    :type filter:  ``str``
    
    :return: The string of source files to insert into Visual Studio
    :rtype:  ``str``
    """
    result = ''
    groups = files[uuid][1]
    for entry in groups:
        if entry[2] in ['all', 'windows'] and os.path.splitext(entry[1])[1] in SOURCE_EXT:
            source = path+'\\'+entry[1]
            if filter:
                result += '\n    <ClCompile Include="__ROOT_DIR__%s">\n      <Filter>Source Files%s</Filter>\n    </ClCompile>'  % (source,path)
            else:
                result += '\n    <ClCompile Include="__ROOT_DIR__%s"/>\n'  % source
        elif entry[2] == 'group':
            localpath = path+'\\'+entry[1]
            result += expand_sources(localpath, entry[0], files, filter)
    return result


def place_project(config):
    """
    Places the Visual Studio project in the build directory
    
    :param config: The project configuration settings
    :type config:  ``dict``
    
    :return: The project (sub)directory
    :rtype:  ``str``
    """
    entries = ['root','build','camel']
    util.check_config_keys(config,entries)
    
    # Create the build folder if necessary
    build = config['build']
    if not os.path.exists(build):
        os.mkdir(build)
    
    # Clear and create the temp folder
    build = util.remake_dir(build,MAKEDIR)
    
    # Name the subdirectory
    project  = os.path.join(build,config['camel'])
    
    # Copy the Visual Studio solution
    template = os.path.join(config['sdl2'],'templates','windows','__project__.sln')
    shutil.copy(template, project+'.sln')
    
    # Copy the include directory
    src = os.path.join(config['sdl2'],'templates','windows','include')
    dst = os.path.join(build,'include')
    shutil.copytree(src, dst, copy_function = shutil.copy)
    
    # Finally, copy in all of the resources
    src = os.path.join(config['sdl2'],'templates','windows','__project__')
    shutil.copytree(src, project, copy_function = shutil.copy)
    
    # We need to rename some files in the subdirectory
    src = os.path.join(project,'__project__.rc')
    dst = os.path.join(project,config['camel']+'.rc')
    shutil.move(src,dst)
    
    src = os.path.join(project,'__project__.props')
    dst = os.path.join(project,config['camel']+'.props')
    shutil.move(src,dst)
    
    src = os.path.join(project,'__project__.vcxproj')
    dst = os.path.join(project,config['camel']+'.vcxproj')
    shutil.move(src,dst)
    
    src = os.path.join(project,'__project__.vcxproj.filters')
    dst = os.path.join(project,config['camel']+'.vcxproj.filters')
    shutil.move(src,dst)

    return project


def reassign_vcxproj(config,project):
    """
    Modifies the Visual Studio project to reflect the current build location
    
    The template Visual Studio project has several variables that begin and end
    with double underscores. The vast  majority of these are directories that
    must be updated to match the current location of the build directory. This
    function updates all of those variables.
    
    :param config: The project configuration settings
    :type config:  ``dict``
    
    :param project: The Windows build directory
    :type project:  ``str``
    """
    # SDL directory (relative)
    sdl2dir = util.path_to_windows(config['build_to_sdl2'])
    sdl2dir = '..\\'+sdl2dir+'\\'
    
    # Source directory (relative)
    rootdir = util.path_to_windows(config['build_to_root'])
    rootdir = '..\\'+rootdir+'\\'
    
    # Asset directory (relative)
    assetdir = util.path_to_windows(config['assets'])
    assetdir = rootdir+assetdir+'\\'
    
    # Compute all the include directories
    make_include = lambda x : '$(GameDir)'+util.path_to_windows(x)
    entries = config['include_dict']
    includes = ''
    if 'all' in entries and entries['all']:
        includes += ';'.join(map(make_include,entries['all']))+';'
    if 'windows' in entries and entries['windows']:
        includes += ';'.join(map(make_include,entries['windows']))+';'
    
    context = {'__project__':config['camel'],'__BUILD_2_SDL__':sdl2dir,'__INCLUDE_DIR__':includes}
    
    # Time to update the files
    solution = project+'.sln'
    util.file_replace(solution,context)
    
    file = os.path.join(project,config['camel']+'.vcxproj')
    util.file_replace(file,context)
    
    file += '.filters'
    util.file_replace(file,context)
    
    # Finally updae the property sheel
    del context['__INCLUDE_DIR__']
    context['__ROOT_DIR__']  = rootdir
    context['__ASSET_DIR__'] = assetdir
    
    file = os.path.join(project,config['camel']+'.props')
    util.file_replace(file,context)
    
    # TODO: Process include directories


def populate_sources(config,project):
    """
    Adds the source code files to the Visual Studio project
    
    This method builds a filetree so that are can organize subdirectories as
    explicit filters in Visual Studio.
    
    :param config: The project configuration settings
    :type config:  ``dict``
    
    :param project: The Windows build directory
    :type project:  ``str``
    """
    filterproj = os.path.join(project,config['camel']+'.vcxproj.filters')
    sourceproj = os.path.join(project,config['camel']+'.vcxproj')
    
    uuidsvc = config['uuids']
    filetree = config['source_tree']
    rootdir = '..\\'+util.path_to_windows(config['build_to_root'])
    if len(config['source_tree']) == 1:
        key = list(config['source_tree'].keys())[0]
        rootdir += '\\'+util.path_to_windows(key)
        filetree = filetree[key]
    
    uuid, files = build_filters(rootdir+'\\',filetree,uuidsvc)
    filters = expand_filters('Source Files', uuid, files)
    
    rootdir = '..\\'+rootdir
    headers = expand_headers('', uuid, files)
    headers = headers.replace('__ROOT_DIR__',rootdir)
    
    sources = expand_sources('', uuid, files)
    sources = sources.replace('__ROOT_DIR__',rootdir)
    
    context = {'__FILTER_ENTRIES__' : filters, '__SOURCE_ENTRIES__' : sources, '__HEADER_ENTRIES__' : headers}
    util.file_replace(filterproj,context)
    
    headers = expand_headers('', uuid, files, False)
    headers = headers.replace('__ROOT_DIR__',rootdir)
    
    sources = expand_sources('', uuid, files, False)
    sources = sources.replace('__ROOT_DIR__',rootdir)
    
    context = {'__SOURCE_ENTRIES__' : sources, '__HEADER_ENTRIES__' : headers}
    util.file_replace(sourceproj,context)
    
    return


def make(config):
    """
    Creates the Vsual Studio project
    
    This only creates the Visual Studio project; it does not actually build the
    project. To build the project, you must open it up in Visual Studio.
    
    :param config: The project configuration settings
    :type config:  ``dict``
    """
    print()
    print('Configuring Windows build files')
    print('-- Copying Visual Studio project')
    project = place_project(config)
    print('-- Modifying project settings')
    reassign_vcxproj(config,project)
    print('-- Populating project file')
    populate_sources(config,project)
    
    if 'icon' in config:
        print('-- Generating icons')
        # Remove any existing icon (as this causes problems)
        icon = os.path.join(project,'icon1.ico')
        os.remove(icon)
        config['icon'].gen_windows(project)
