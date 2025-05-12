"""
Python Script for Windows Builds

While technically Windows can run CMake, we find CMake support to be flaky. In
particular, the optional formats in sdl2image break CMake because Microsoft
MSVC compiler does not support stdatomic.h in C (sigh). Therefore, we prefer
to use Visual Studio for Windows builds.

This current version includes extensions allowing students to incorporate
third-party libraries. However, this functionality is limited as it does not
include support for Win32/x86 projects like the rest of our engine. It can only
handle x64 projects. This is because CMake is also extremely unreliable for
finding the correct libraries for 32 bit projects.

Author:  Walker M. White
Version: 7/10/24
"""
import xml.etree.ElementTree as ET
import os, os.path
import subprocess
import shutil
import glob
import copy
from . import util

# The subbuild folder for the project
MAKEDIR = 'windows'

# Supported header extensions
HEADER_EXT = ['.h', '.hh', '.hpp', '.hxx', '.hm', '.inl', '.inc', '.xsd']
# Supportes source extensions
SOURCE_EXT = ['.cpp', '.c', '.cc', '.cxx', '.m', '.mm', '.def', '.odl', '.idl', '.hpj', '.bat', '.asm', '.asmx']

# The UUIDS for solution integration
PROJ_UUID = '8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942'
CONT_UUID = '2150E333-8FDC-42A3-9474-1A3956D46DE8'

# Project UUIDs that must be replaced
UUID1 = 'AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAA01'

# The module project IDs
MODULES = {
    'audio' : ('cugl-audio','F8748A4B-09DA-4738-BA8E-A803B972B2DA'),
    'core' : ('cugl-core','1EF11169-0BB7-438F-A0A8-E091DDABA512'),
    'graphics' : ('cugl-graphics','B6B90309-0F84-4720-8C57-B4F07D0148E1'),
    'physics2' : ('cugl-physics2','D760AAB9-E462-4798-B80F-F84A9AE30540'),
    'netcode' : ('cugl-netcode','9C944826-FC11-43A6-A70D-4B5507542E39'),
    'physics2::distrib' : ('cugl-distrib-physics2','0B582C4D-2886-494D-A585-1E90152E337F'),
    'scene2' : ('cugl-scene2','DE320596-02AB-43D4-A9D4-C7D7C4BF9749'),
    'scene3' : ('cugl-scene3','1BD80952-C4C8-4752-8AF6-799139FA6422'),
}


def filter_vslist(text,prefixes=None):
    """
    Returns a list of elements in a Visual Studio component list.

    Visual Studio component lists are semi-colon separated. This function
    ignores environment variables (e.g. values starting with '%'). It also
    ignores any elements that have one of the given prefixes.

    :param text: The text to extract from
    :type text:  ``str``

    :param prefixes: The prefixes of elements to ignore
    :type prefixes:  ``list`` of ``str``

    :return: A list of elements in a Visual Studio component list.
    :rtype:  ``list`` of ``str``
    """
    result = []
    items = text.split(';')
    for elt in items:
        if len(elt) == 0 or elt[0] == '%':
            pass
        elif prefixes:
            bad = False
            for test in prefixes:
                if elt.startswith(test):
                    bad = True
                    break
            if not bad:
                result.append(elt)
        else:
            result.append(elt)
    return result


def parse_solution(solution):
    """
    Returns a dictionary composed of Visual Studio information

    Unlike vcxproj files, solutions are NOT in XML format. Instead, they are
    proprietary files broken into three sections: the header, the included
    projects, and the global grouping information. This function returns a
    dictionary of the contents of each section.

    :param solution: The path to the Visual Studio solution
    :type solution:  ``str``

    :return: The dictionary of Visual Studio information
    :rtype:  ``dict``
    """
    contents = {}
    with open(solution) as file:
        state = 'SLNHeader'
        header = []
        project = None
        globals = None
        contents[state] = header
        check = 0

        dependency = False
        for line in file:
            check += 1
            if 'Project(' in line:
                if state == 'SLNHeader':
                    state = 'SLNProjects'
                    contents[state] = {}

                pos0 = line.find('Project("')
                pos0 = line.find('{',pos0)+1
                pos1 = line.find('}',pos0)
                hook = line[pos0:pos1]

                pos0 = line.find('=',pos1)
                pos0 = line.find('"',pos0)+1
                pos1 = line.find('"',pos0)
                name = line[pos0:pos1]

                pos0 = line.find(',',pos1)
                pos0 = line.find('"',pos0)+1
                pos1 = line.find('"',pos0)
                path = line[pos0:pos1]

                pos0 = line.find(',',pos1)
                pos0 = line.find('{',pos0)+1
                pos1 = line.find('}',pos0)
                uuid = line[pos0:pos1]

                project = {}
                project['path'] = path
                project['name'] = name
                project['hook'] = hook
                project['dependencies'] = []

                contents[state][uuid] = project
            elif 'Global' == line.strip():
                state = 'SLNGlobals'
                contents[state] = {}
            elif state == 'SLNHeader':
                header.append(line[:-1])
            elif state == 'SLNProjects':
                if 'ProjectSection(ProjectDependencies)' in line:
                    dependency = True
                elif 'EndProjectSection' in line:
                    dependency = False
                elif 'EndProject' in line:
                    project = None
                elif dependency:
                    pos0 = line.find('{')+1
                    pos1 = line.find('}',pos0)
                    project['dependencies'].append(line[pos0:pos1])
            elif state == 'SLNGlobals':
                if globals is None and 'GlobalSection' in line:
                    pos0 = line.find('(')+1
                    pos1 = line.find(')')
                    stage = line[pos0:pos1]
                    pos0 = line.find('=',pos1)+1
                    phase = line[pos0:].strip()
                    globals = []
                    contents[state][stage] = {}
                    contents[state][stage]['phase'] =phase
                    contents[state][stage]['values'] = globals
                elif 'EndGlobalSection' in line:
                    globals = None
                elif not globals is None:
                    globals.append(line[:-1])

    return contents


def write_solution(solution,solndata,dependencies=True):
    """
    Recollates the solution data back into a Visual Studio solution file

    The optional dependencies attribute is because solution dependencies are
    not strickly necessary. The projects have that information too.

    :param solution: The path to the application solution
    :type solution:  ``str`

    :param solndata: The parsed solution data
    :type solndata:  ``dict`

    :param depedencies: Whether to write out project dependencies in the solution
    :type depedencies:  ``bool``
    """
    with open(solution,'w') as file:
        for item in solndata['SLNHeader']:
            file.write(item)
            file.write('\n')
        for uuid in solndata['SLNProjects']:
            entry = solndata['SLNProjects'][uuid]
            file.write('Project("{')
            file.write(entry['hook'])
            file.write('}") = "')
            file.write(entry['name'])
            file.write('", "')
            file.write(entry['path'])
            file.write('", "{')
            file.write(uuid)
            file.write('}"\n')
            if dependencies and entry['hook'] != CONT_UUID:
                file.write('\tProjectSection(ProjectDependencies) = postProject\n')
                for guid in entry['dependencies']:
                    file.write('\t\t{')
                    file.write(guid)
                    file.write('} = {')
                    file.write(guid)
                    file.write('}\n')
                file.write('\tEndProjectSection\n')
            file.write('EndProject\n')
        file.write('Global\n')
        for section in solndata['SLNGlobals']:
            file.write('\tGlobalSection(')
            file.write(section)
            file.write(') = ')
            file.write(solndata['SLNGlobals'][section]['phase'])
            file.write('\n')
            for v in solndata['SLNGlobals'][section]['values']:
                file.write(v)
                file.write('\n')
            file.write('\tEndGlobalSection\n')
        file.write('EndGlobal\n')


def write_project(project, doctree):
    """
    Writes the XML for a vcxproj to a file.

    While vcxproj files are in XML, the format is slightly different than the
    one produced by Python's ElementTree. Therefore, we have to do a little
    bit of processing before we write it to a file.

    :param project: The path to the vcxproj file
    :type project:  ``str``

    :param doctree: The project XML structure
    :type doctree:  ``ElementTree``
    """
    root = doctree.getroot()

    default_target = 'Build'
    if "DefaultTargets" in root.attrib:
        default_target = root.attrib['DefaultTargets']
    tools_version = '17.0'
    if "ToolsVersion" in root.attrib:
        tools_version = root.attrib['ToolsVersion']
    namespace = 'http://schemas.microsoft.com/developer/msbuild/2003'
    if '}' in root.tag:
        pos = root.tag.find('}')
        namespace = root.tag[1:pos]

    # Fix python weirdness
    xml = ET.tostring(root).decode('utf-8')
    pos0 = xml.find('<')+1
    pos1 = xml.find(':',pos0)
    prefix = xml[pos0:pos1]

    # Fix the python namespace manglng and put in standard form
    xml = xml.replace('<%s:' % prefix,'<')
    xml = xml.replace('</%s:' % prefix,'</')

    prefix  = '<?xml version="1.0" encoding="utf-8"?>\n'
    prefix += '<Project DefaultTargets="%s" ToolsVersion="%s" xmlns="%s">' % (default_target,tools_version,namespace)
    pos = xml.find('\n')
    xml = prefix + xml[pos:]

    with open(project,'w') as file:
        file.write(xml)


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


def make_directory(config):
    """
    Creates the build directory for Apple projects

    This also copies over a private copy of the cugl-core VS project so we can
    do some optimizations (all other projects can be supported in place).

    :param config: The project configuration settings
    :type config:  ``dict``
    """
    entries = ['root','build','build_to_cugl']
    util.check_config_keys(config,entries)

    # Create the build folder if necessary
    build = config['build']
    if not os.path.exists(build):
        os.mkdir(build)

    # Clear and create the temp folder
    build = util.remake_dir(build,MAKEDIR)

    # Create a cugl-core subfolder if necessary
    coredir = os.path.join(build,'cugl-core')
    if not os.path.exists(coredir):
        os.mkdir(coredir)

    # Copy the cugl-core project
    src = os.path.join(config['cugl'],'buildfiles',MAKEDIR,'cugl-core','cugl-core.vcxproj')
    dst = os.path.join(coredir,'cugl-core.vcxproj')
    shutil.copyfile(src, dst)

    src = os.path.join(config['cugl'],'buildfiles',MAKEDIR,'cugl-core','cugl-core.vcxproj.filters')
    dst = os.path.join(coredir,'cugl-core.vcxproj.filters')
    shutil.copyfile(src, dst)

    src = os.path.join(config['cugl'],'buildfiles',MAKEDIR,'cugl-core','cugl-core.vcxproj.user')
    dst = os.path.join(coredir,'cugl-core.vcxproj.user')
    shutil.copyfile(src, dst)

    # Fix the references in these files
    contents = dict()
    contents['..\\source\\core'] = config['build_to_cugl']+'\\source\\core'
    contents['..\\include\\cugl\\core'] = config['build_to_cugl']+'\\include\\cugl\\core'
    contents['$(ProjectDir)..\\include'] = '$(ProjectDir)..\\..\\'+config['build_to_cugl']+'\\buildfiles\\windows\\include'
    contents['$(ProjectDir)..\\..\\..\\include'] = '$(ProjectDir)..\\..\\'+config['build_to_cugl']+'\\include'
    contents['$(ProjectDir)..\\..\\..\\sdlapp'] = '$(ProjectDir)..\\..\\'+config['build_to_cugl']+'\\sdlapp'
    contents['$(ProjectDir)..\\..\\..\\external'] = '$(ProjectDir)..\\..\\'+config['build_to_cugl']+'\\external'
    contents['Include="..\\..\\..\\sdlapp'] = 'Include="..\\..\\'+config['build_to_cugl']+'\\sdlapp'
    contents['Include="poly2tri'] = 'Include="..\\..\\'+config['build_to_cugl']+'\\buildfiles\\windows\\cugl-core\\poly2tri'
    if config['headless']:
        contents['CU_USE_OPENGL'] = 'CU_HEADLESS'

    source = os.path.join(coredir,'cugl-core.vcxproj')
    util.file_replace(source,contents)

    source = os.path.join(coredir,'cugl-core.vcxproj.filters')
    util.file_replace(source,contents)


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
    build = os.path.join(config['build'],MAKEDIR)
    if not os.path.exists(build):
        os.mkdir(build)

    # Name the subdirectory
    project  = os.path.join(build,config['camel'])

    # Copy the Visual Studio solution
    template = os.path.join(config['cugl'],'templates','windows','__project__.sln')
    shutil.copy(template, project+'.sln')

    # Copy the include directory
    src = os.path.join(config['cugl'],'templates','windows','include')
    dst = os.path.join(build,'include')
    shutil.copytree(src, dst, copy_function = shutil.copy)

    # Finally, copy in all of the resources
    src = os.path.join(config['cugl'],'templates','windows','__project__')
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
    function updates most of those variables.

    Note that while the vcxproj is an XML file, it is generally easier to
    treat it as a straight file for simple, global replacements. That is why
    this function does not substitute configuration specific values. Those
    are handled later in expand_build_configurations as necessary.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param project: The Windows build directory
    :type project:  ``str``
    """
    # CUGL directory (relative)
    cugldir = util.path_to_windows(config['build_to_cugl'])
    cugldir = '..\\'+cugldir+'\\'

    # Source directory (relative)
    rootdir = util.path_to_windows(config['build_to_root'])
    rootdir = '..\\'+rootdir+'\\'

    # Asset directory (relative)
    assetdir = util.path_to_windows(config['assets'])
    assetdir = rootdir+assetdir+'\\'

    context = {'__project__':config['camel'],'__BUILD_2_CUGL__':cugldir}

    # Need to replace one UUID1
    uuidsvc = config['uuids']
    uuid = uuidsvc.getWindowsUUID('PROJECT:\\\\'+config['camel'])
    config['windows'] = {}
    config['windows']['uuid'] = uuid
    context[UUID1] = uuid

    # Time to update the files
    solution = project+'.sln'
    util.file_replace(solution,context)

    file = os.path.join(project,config['camel']+'.vcxproj')
    util.file_replace(file,context)

    file += '.filters'
    util.file_replace(file,context)

    # Finally updae the property sheel
    context['__ROOT_DIR__']  = rootdir
    context['__ASSET_DIR__'] = assetdir

    file = os.path.join(project,config['camel']+'.props')
    util.file_replace(file,context)


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


def filter_modules(config,project):
    """
    Assigns the correct modules to the project solution.

    The latest version of CUGL breaks it up in the modules to speed up the
    build process. This removes any unnecessary modules from the link phase.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param project: The Windows build directory
    :type project:  ``str``
    """
    cugldir = util.path_to_windows(config['build_to_cugl'])
    preamble = '    <ProjectReference Include="..\\..\\'+cugldir+'\\buildfiles\\windows\\'
    middle = '">\n      <Project>{'
    ending = '}</Project>\n    </ProjectReference>\n'

    libs = ['core']
    for key in config['modules']:
        if config['modules'][key]:
            libs.append(key)

    links = ''
    for item in libs:
        name, uuid = MODULES[item]
        links += preamble
        links += name+'\\'+name+'.vcxproj'
        links += middle
        links += uuid
        links += ending

    sourceproj = os.path.join(project,config['camel']+'.vcxproj')
    context = {'__CUGL_MODULES__\n' : links}
    util.file_replace(sourceproj,context)


def expand_global_configurations(config,project):
    """
    Assigns the includes and defines to the application project.

    This function is a simpler form of expand_build_configurations. If we do
    not need different settings for each build, we simply due a global replace
    on the file without having to parse an XML structure.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param project: The Windows build directory
    :type project:  ``str``
    """
    file = os.path.join(project,config['camel']+'.vcxproj')

    # Get the base line configs and defines
    make_include = lambda x : '$(GameDir)'+util.path_to_windows(x)
    entries = config['include_dict']
    includes = []
    if 'all' in entries and entries['all']:
        includes += entries['all']
    if 'windows' in entries and entries['windows']:
        includes += entries['windows']
    if includes:
        includes = ';'.join(map(make_include,allincludes))+';'
    else:
        includes = ''

    entries = config['define_dict']
    defines = []
    if 'all' in entries and entries['all']:
        defines += entries['all']
    if 'windows' in entries and entries['windows']:
        defines += entries['windows']
    if defines:
        defines = ';'.join(alldefines)+';'
    else:
        defines = ''

    context = {'__INCLUDE_DIR__':includes,'__CUGL_DEFINES__':defines,'__CUGL_DEPENDS__':''}
    util.file_replace(file,context)


# THIS NEXT PART IS THE HEADACHE OF EXTERNAL LIBRARIES
# We only support this on machines running Windows

def expand_build_configurations(config,project):
    """
    Assigns the build infortmation to the application project.

    The build information is the includes, defines, and dependencies for each
    build type. This function is a more fine-grained alternative to
    expand_global_configurations for the cases when we need more fine-tuned
    control (e.g. external module support).

    :param config: The project configuration settings
    :type config:  ``dict``

    :param project: The Windows build directory
    :type project:  ``str``
    """
    file = os.path.join(project,config['camel']+'.vcxproj')

    # Get the base line configs and defines
    make_include = lambda x : '$(GameDir)'+util.path_to_windows(x)
    entries = config['include_dict']
    allincludes = []
    if 'all' in entries and entries['all']:
        allincludes += entries['all']
    if 'windows' in entries and entries['windows']:
        allincludes += entries['windows']
    if allincludes:
        allincludes = ';'.join(map(make_include,allincludes))+';'
    else:
        allincludes = ''

    entries = config['define_dict']
    alldefines = []
    if 'all' in entries and entries['all']:
        alldefines += entries['all']
    if 'windows' in entries and entries['windows']:
        alldefines += entries['windows']
    if alldefines:
        alldefines = ';'.join(alldefines)+';'
    else:
        alldefines = ''

    builddata = config['windows']['builds']

    doctree = ET.parse(file)
    root = doctree.getroot()
    builddefs = root.findall("./{*}ItemDefinitionGroup[@Condition]")
    for build in builddefs:
        text = build.attrib['Condition']
        pos0 = text.find('==')
        pos0 = text.find("'",pos0)+1
        pos1 = text.find("|",pos0)
        kind = text[pos0:pos1].lower()
        pos0 = pos1+1
        pos1 = text.find("''",pos0)
        arch = text[pos0:pos1].lower()

        # Gather the build specific informatation
        if arch == 'x64' and builddata[kind]['includes']:
            includes = allincludes+';'.join(builddata[kind]['includes'])+';'
        else:
            includes = allincludes
        if  arch == 'x64' and builddata[kind]['defines']:
            defines = alldefines+';'.join( builddata[kind]['defines'])+';'
        else:
            defines = ''
        if arch == 'x64' and builddata[kind]['dependencies']:
            depends  = ';'.join(builddata[kind]['dependencies'])+';'
        else:
            depends = ''

        includeset = build.findall("./{*}ClCompile/{*}AdditionalIncludeDirectories")
        for entry in includeset:
            entry.text = entry.text.replace('__INCLUDE_DIR__',includes)

        defineset = build.findall("./{*}ClCompile/{*}PreprocessorDefinitions")
        for entry in defineset:
            entry.text = entry.text.replace('__CUGL_DEFINES__',defines)

        dependset = build.findall("./{*}Link/{*}AdditionalDependencies")
        for entry in dependset:
            entry.text = entry.text.replace('__CUGL_DEPENDS__',depends)

    # This existenace of this infomation implies external modules
    if 'external' in config:
        refset = root.find("./{*}ItemGroup/{*}ProjectReference[@Include]/..")
        namespace = 'http://schemas.microsoft.com/developer/msbuild/2003'
        if '}' in root.tag:
            pos = root.tag.find('}')
            namespace = root.tag[1:pos]

        dependencies = []
        for module in config['external']:
            if 'windows' in module:
                for puuid in module['windows']:
                    project = module['windows'][puuid]
                    name = project['name']
                    path = module['name']+'\\'+project['path']
                    dependencies.append((puuid,name,path))

        for item in dependencies:
            pref_elmnt = ET.SubElement(refset,'{%s}ProjectReference' % namespace)
            pref_elmnt.set('Include',item[2])
            uuid_elmnt = ET.SubElement(pref_elmnt,'{%s}Project' % namespace)
            uuid_elmnt.text = '{%s}' % item[0]
            name_elmnt = ET.SubElement(pref_elmnt,'{%s}Name' % namespace)
            name_elmnt.text = item[1]

        ET.indent(doctree, space="\t", level=0)

    write_project(file,doctree)


def expand_targets(target,targetset,solndata):
    """
    Recursively identifies the vcxproj projects for each target.

    This function finds the project matching target (which can be a name or a
    project UUID) in solndata.  If it finds it, it adds it to targetset, which
    is a dictionary of project information keyed on project UUID.

    It also looks for any dependencies of this project (other than ZERO_CHECK).
    It recursively calls the function on those dependencies to add them to the
    targetset.

    :param target: A target name or project UUID
    :type target:  ``str``

    :param targetset: A dictionary maping project UUIDs to project info
    :type targetset:  ``dict``

    :param solndata: The information from the generated solution file
    :type solndata:  ``dict``
    """
    if not 'SLNProjects' in solndata:
        return
    elif target in targetset:
        return

    uuid = None
    dependencies = []
    section = solndata['SLNProjects']
    for item in section:
        if target == item or target == section[item]['name']:
            if section[item]['name'] == 'ZERO_CHECK':
                return

            uuid = item
            targetset[uuid] = copy.deepcopy(section[item])
            del targetset[uuid]['dependencies']
            for elt in section[item]['dependencies']:
                dependencies.append(elt)

    if uuid is None:
        return

    for elt in dependencies:
        expand_targets(elt,targetset,solndata)


def extract_module_projects(module,solution):
    """
    Returns a dictionary of project files for the given module.

    This function opens the solutiuon identify ALL the projects created for
    this module. For each such project, it grabs the UUID, name, and path.
    This information is then stored in a dictionary keyed on project UUID.
    :param config: The project configuration settings
    :type config:  ``dict``

    :param module: The module information
    :type project:  ``dict``

    :param solution: The path to the generated solution
    :type solution:  ``str``

    :return: A dictionary of project files for the given module.
    :rtype:  ``dict``
    """
    if not 'targets' in module:
        return

    pos = solution.rfind('.')
    if pos == -1:
        return

    targetset = {}
    solndata = parse_solution(solution)
    for target in module['targets']:
        expand_targets(target,targetset,solndata)

    # Strip out all extraneous projects
    deleteset = {}
    for item in solndata['SLNProjects']:
        if not item in targetset:
            deleteset[item] = True

    for uuid in deleteset:
        del solndata['SLNProjects'][uuid]

    for uuid in solndata['SLNProjects']:
        dependencies = []
        for item in solndata['SLNProjects'][uuid]['dependencies']:
            if not item in deleteset:
                dependencies.append(item)
        solndata['SLNProjects'][uuid]['dependencies'] = dependencies

    if 'ProjectConfigurationPlatforms' in solndata['SLNGlobals']:
        group = solndata['SLNGlobals']['ProjectConfigurationPlatforms']
        values = []
        for item in group['values']:
            pos0 = item.find('{')+1
            pos1 = item.find('}',pos0)
            guid = item[pos0:pos1]
            if not guid in deleteset:
                values.append(item)
        group['values'] = values

    if 'NestedProjects' in solndata['SLNGlobals']:
        del solndata['SLNGlobals']['NestedProjects']

    write_solution(solution,solndata)
    return targetset


def reassign_module_vcxprojs(module,path):
    """
    Reassigns the entries in the generated vcxproj files

    The purpose of this function is to clean up any CMake weirdness in the
    generated project files. The primary thing it does is remove references to
    ZERO_CHECK.

    :param module: The module information
    :type project:  ``dict``

    :param path: The path to the generated projects
    :type path:  ``dict``
    """
    if not 'windows' in module:
        return

    projset = module['windows']
    for puuid in module['windows']:
        vcxproj = os.path.join(path, module['windows'][puuid]['path'])
        doctree = ET.parse(vcxproj)
        root = doctree.getroot()

        # Remove the ZERO_CHECKs
        refset = root.find("./{*}ItemGroup/{*}ProjectReference[@Include]/..")
        zeroset = refset.findall("./{*}ProjectReference[@Include]/{*}Name[.='ZERO_CHECK']/..")
        for item in zeroset:
            refset.remove(item)

        write_project(vcxproj, doctree)


def extract_module_data(module,path):
    """
    Returns a configuration dictionary for linking in an external project

    Ths function takes the generated main project for the given module, and
    gathers all of the include data, define data, and dependencies for each
    build type (e.g. 'debug', 'release'). This information is stored in a
    dictionary keyed on build type. This is the dictionary returned by this
    function.

    :param module: The module information
    :type project:  ``dict``

    :param path: The path to the generated project
    :type path:  ``dict``

    :return: A configuration dictionary for linking in an external project
    :rtype:  ``dict``
    """
    vcxproj = os.path.join(path,'Main.vcxproj')
    if not os.path.isfile(vcxproj):
        return {}

    doctree = ET.parse(vcxproj)
    root = doctree.getroot()

    result = {}
    builddefs = root.findall("./{*}ItemDefinitionGroup[@Condition]")
    for build in builddefs:
        text = build.attrib['Condition']
        pos0 = text.find('==')
        pos0 = text.find("'",pos0)+1
        pos1 = text.find("|",pos0)
        kind = text[pos0:pos1].lower()

        if not kind in result:
            result[kind] = {}
            result[kind]['includes'] = []
            result[kind]['defines'] = []
            result[kind]['dependencies'] = []

        includeset = build.findall("./{*}ClCompile/{*}AdditionalIncludeDirectories")
        for entry in includeset:
            result[kind]['includes'].extend(filter_vslist(entry.text))
        result[kind]['includes'] = list(set(result[kind]['includes']))

        defineset = build.findall("./{*}ClCompile/{*}PreprocessorDefinitions")
        excepts = ['WIN32','_WINDOWS','CMAKE_INTDIR','NDEBUG','HAVE_CONFIG_H','_CRT_']
        for entry in defineset:
            result[kind]['defines'].extend(filter_vslist(entry.text,excepts))
        result[kind]['defines'] = list(set(result[kind]['defines']))

        dependset = build.findall("./{*}Link/{*}AdditionalDependencies")
        excepts = ['kernel32', 'user32', 'gdi32', 'winspool', 'shell32', 'ole32', 'oleaut32', 'uuid', 'comdlg32', 'advapi32']
        for entry in dependset:
            result[kind]['dependencies'].extend(filter_vslist(entry.text,excepts))
        result[kind]['dependencies'] = list(set(result[kind]['dependencies']))

        # Filter out the internal dependencies
        actual = []
        for item in result[kind]['dependencies']:
            if not '\\' in item or ':' in item:
                actual.append(item)

        actual.sort()
        result[kind]['dependencies'] = actual

    return result


def configure_external_module(config,module,solution):
    """
    Returns a configuration dictionary for linking in an external project

    This function does to thinks. First of all, it uses the generated solution
    to identify ALL the projects created for this module. For each such project,
    it grabs the UUID, name, and path.  This information is then stored as a
    dictionary in module['windows'].

    Then it goes to the shiv project and gathers all of the include data,
    define data, and dependencies for each build type (e.g. 'debug', 'release').
    This information is stored in a dictionary keyed on build type. This is the
    dictionary returned by this function.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param module: The module information
    :type project:  ``dict``

    :param solution: The path to the generated solution
    :type solution:  ``str``

    :return: A configuration dictionary for linking in an external project
    :rtype:  ``dict``
    """
    if not 'targets' in module:
        return None

    uuidsvc = config['uuids']

    # Unforunately, this is not in XML
    module['windows'] = extract_module_projects(module,solution)
    path = os.path.split(solution)[0]
    reassign_module_vcxprojs(module,path)
    data = extract_module_data(module,path)
    return data


def install_cmake(config,module,projdir):
    """
    Creates a new directory for the module and copies in the CMake shiv.

    The CMake shiv is necessary because Windows CMake will drop important
    linking information if we do not actually link the module to an application.
    Fortunately, it is enough to link to a Hello World application, which is
    what we do. This shiv can then be used to create the necessary project
    files.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param module: The module information
    :type project:  ``dict``

    :param projdir: The folder containing the application vcxproj
    :type projdir:  ``str``
    """
    moduledir = os.path.join(projdir,module['name'])
    if not os.path.exists(moduledir):
        os.mkdir(moduledir)

    shivdir = os.path.join(moduledir,'bridge')
    tooldir = os.path.join(config['cugl'],'tools','windows')
    shutil.copytree(tooldir,shivdir,dirs_exist_ok=True)

    cmake = os.path.join(shivdir, 'CMakeLists.txt')
    prefix = '../../../..'

    # Set CUGL defines
    context = {}

    # Set the externals
    prefix = '${PROJECT_SOURCE_DIR}/'+prefix+'/'+util.path_to_posix(config['build_to_project'])
    externals  = "# External package %s\n" % repr(module['name'])
    externals += 'add_subdirectory("%s/%s" "lib")\n' % (prefix,module['path'])
    if 'targets' in module:
        for target in module['targets']:
            externals += 'list(APPEND EXTRA_LIBS %s)\n' % target

    context['__EXTERNALS__'] = externals
    util.file_replace(cmake,context)
    return moduledir


def add_external_module(config,module,moduledir):
    """
    Adds any external modules to the application.

    This function extracts any necessary information necessary for later
    configuring the builds with expand_build_configurations. This information
    is stored in config['windows']['builds'].

    :param config: The project configuration settings
    :type config:  ``dict``

    :param module: The module information
    :type project:  ``dict``

    :param moduledir: The folder to contain the module build files
    :type moduledir:  ``str``
    """
    # Create a macos subfolder if necessary
    # Set ip the CMake
    cwd = os.getcwd()
    os.chdir(moduledir)
    path = os.path.join(moduledir,'bridge')

    command = ['cmake']
    if 'options' in module:
        for opt in module['options']:
            command.append('-D'+opt+'='+str(module['options'][opt]))

    # Note the is x64 only.
    # We tried to include Win32/x86 as well, but it is not really possible.
    # The problem is that when CMake searches for 3rd pary libraries, it cannot
    # tell  the difference between Win32 and x64 libraries.
    command.append('-G Visual Studio 17 2022')
    command.append('-A x64')
    command.append(path)
    subprocess.run(command)

    # Locate the visual studeio solution
    solution = glob.glob(os.path.join(moduledir,'*.sln'))
    if (len(solution) == 0):
        print('   ERROR: No Visual Studio solution generated for %s' % repr(module['name']))
        solution = None
    elif (len(solution) > 1):
        print('   WARNING: Found multiple Visual Studio solutions for %s' % repr(module['name']))
        solution = solution[0]
    else:
        solution = solution[0]

    # If the project exists, link it up
    if solution:
        builds = configure_external_module(config,module,solution)
        if not 'windows' in config:
            config['windows'] = {}
        if not 'builds' in config['windows']:
            config['windows']['builds'] = {}
        config['windows']['builds'] = util.merge_dict(builds,config['windows']['builds'])

    # Restore the working directory
    os.chdir(cwd)


def integrate_external_module(config,module,solndir):
    """
    Integrates the module projects into the root solution.

    Even though we have integrated the projects into the application vcxproj
    file, Visual Studion will not reliably build these projects unless we add
    them to the solution as well. The relevant information will be stored in
    the 'windows' value of module.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param module: The module information
    :type project:  ``dict``

    :param solndir: The folder containing the application solution
    :type project:  ``str``
    """
    if not 'windows' in module:
        return
    elif not 'windows' in config or not 'uuid' in config['windows']:
        return

    solution = os.path.join(solndir,config['camel']+'.sln')
    solndata = parse_solution(solution)

    uuidsvc = config['uuids']
    uuid = uuidsvc.getWindowsUUID('GROUP:\\\\'+module['name'])
    uuid = uuidsvc.applyPrefix('DA',uuid)
    group = 'lib'+module['name']
    if group.lower() == config['camel'].lower():
        group = module['name']

    solndata['SLNProjects'][uuid] = {}
    solndata['SLNProjects'][uuid]['hook'] = CONT_UUID
    solndata['SLNProjects'][uuid]['name'] = group
    solndata['SLNProjects'][uuid]['path'] = group
    solndata['SLNProjects'][uuid]['dependencies'] =[]

    projset = module['windows']
    for puuid in projset:
        project = projset[puuid]
        solndata['SLNProjects'][puuid] = {}
        solndata['SLNProjects'][puuid]['hook'] = PROJ_UUID
        solndata['SLNProjects'][puuid]['name'] = project['name']
        solndata['SLNProjects'][puuid]['path'] = config['camel']+'\\'+module['name']+'\\'+project['path']
        solndata['SLNProjects'][puuid]['dependencies'] = []

        section = solndata['SLNGlobals']['ProjectConfigurationPlatforms']['values']
        section.append('\t\t{%s}.Debug|x64.ActiveCfg = Debug|x64' % puuid)
        section.append('\t\t{%s}.Debug|x64.Build.0 = Debug|x64' % puuid)
        section.append('\t\t{%s}.Release|x64.ActiveCfg = Release|x64' % puuid)
        section.append('\t\t{%s}.Release|x64.Build.0 = Release|x64' % puuid)

        section = solndata['SLNGlobals']['NestedProjects']['values']
        section.append('\t\t{%s} = {%s}' % (puuid, uuid))

    # Add all of these as depeendencies to the main class.
    # Means we always build twice (anything else is too brittle)
    theuuid = config['windows']['uuid']
    if theuuid in solndata['SLNProjects']:
        dependencies = solndata['SLNProjects'][theuuid]['dependencies']
        projset = module['windows']
        for puuid in projset:
            dependencies.append(puuid)

    write_solution(solution,solndata)


def find_nonstandard_dlls(config):
    """
    Returns the DLLs required by any external modules

    On Windows, CMake may link against some static libraries that are really
    just an interface to a DLL. This function attempts to define such libraries.
    It cannot actually find the libraries, as they can be anywhere in the
    user's library path. But we can at least warn the user.

    :param config: The project configuration settings
    :type config:  ``dict``

    :return: The DLLs required by any external modules
    :rtype:  ``list`` of ``str``
    """

    libfiles = []
    if 'windows' in config and 'builds' in config['windows']:
        for kind in config['windows']['builds']:
            if 'dependencies' in config['windows']['builds'][kind]:
                for file in config['windows']['builds'][kind]['dependencies']:
                    if ':\\' in file:
                        libfiles.append(file)
    dlls = []
    for item in libfiles:
        if '.dll' in item:
            dlls.append(item)
        else:
            with open(item,'rb') as file:
                for line in file:
                    if b'.dll' in line:
                        pos0 = 0
                        while pos0 < len(line):
                            pos0 = line.find(b'.dll',pos0)
                            if pos0 == -1:
                                pos0 = len(line)
                            else:
                                pos1 = pos0
                                found = False
                                while pos1 > 0 and not found:
                                    if line[pos1] > 31 and line[pos1] < 127:
                                        pos1 -= 1
                                    else:
                                        pos1 += 1
                                        found = True
                                dlls.append(line[pos1:pos0+4])
                                pos0 += 4

    dlls = list(set(map(lambda x: x.decode('utf-8'), dlls)))
    dlls.sort()
    return dlls


def add_externals(config,projdir):
    """
    Adds any external modules to this Visual Studio project

    :param config: The project configuration settings
    :type config:  ``dict``

    :param buildir: The Windows build directory
    :type buildir:  ``str``
    """
    if not 'external' in config:
        return False

    if util.get_dev_platform() != 'windows':
        print('   [SKIP]: External libraries require Windows for Visual Studio integration')
        return False

    for module in config['external']:
        print('-- Expanding %s' % repr(module['name']))
        moduledir = install_cmake(config,module,projdir)
        add_external_module(config,module,moduledir)
        integrate_external_module(config,module,os.path.split(projdir)[0])

    # Finish off wtih some warnings
    print('  [WARNING] External modules support is x64 only')

    # Look for any nonstandard dlls
    dlls = find_nonstandard_dlls(config)
    if dlls:
        pream = 'These modules depend' if len(config['external']) > 1 else 'This module depends'
        print('  [WARNING] %s on external dlls' % pream)
        for item in dlls:
            print('  - '+item)
        print("  Put these in CUGL's buidlfiles\\windows\\dlls\\x64 directory")

    return True


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
    make_directory(config)
    project = place_project(config)
    print('-- Modifying project settings')
    reassign_vcxproj(config,project)
    print('-- Populating project file')
    populate_sources(config,project)
    print('-- Configuring modules')
    filter_modules(config,project)
    if add_externals(config,project):
        expand_build_configurations(config,project)
    else:
        expand_global_configurations(config,project)

    if 'icon' in config:
        print('-- Generating icons')
        # Remove any existing icon (as this causes problems)
        icon = os.path.join(project,'icon1.ico')
        os.remove(icon)
        config['icon'].gen_windows(project)
