"""
Python Script for Android Builds

Behold the entire reason we have a custom build set-up for CUGL. While CMake conceivably
works with iOS (though not well), it does not work with Android at all. That is because
an Android project is an amalgamation of C++ files, Java files, Makefiles and Gradle
files. Configuring these projects is error prone, as a lot of different files have to
be touched.

Author: Walker M. White
Date:   11/29/22
"""
import os, os.path
import shutil
import subprocess
import platform
import re, string
from . import util

# The subbuild folder for the project
MAKEDIR = 'android'

# Supported header extensions
HEADER_EXT = ['.h', '.hh', '.hpp', '.hxx', '.hm']
# Supportes source extensions
SOURCE_EXT = ['.cpp', '.c', '.cc', '.cxx', '.asm', '.asmx']


def expand_sources(path, filetree):
    """
    Returns the string of source files to insert into Android.mk
    
    This string should replace __SOURCE_FILES__ in the makefile.
    
    :param path: The path to the root directory for the filters
    :type path:  ``str1``
    
    :param filetree: The file tree storing both files and filters
    :type filetree:  ``dict``
    
    :return: The string of source files to insert into Android.mk
    :rtype:  ``str``
    """
    result = ''
    for key in filetree:
        # Recurse on directories
        if type(filetree[key]) == dict:
            result += expand_sources(path+'/'+key,filetree[key])
        else:
            category = filetree[key]
            if category in ['all', 'android'] and os.path.splitext(key)[1] in SOURCE_EXT:
                result += ' \\\n\t%s/%s' % (path,key)
    return result


def expand_includes(path, filetree):
    """
    Returns a set of directories to add to Android.mk for inclusion
    
    :param path: The path to the root directory for the filters
    :type path:  ``str1``
    
    :param filetree: The file tree storing both files and filters
    :type filetree:  ``dict``
    
    :return: A set of directories to add to Android.mk for inclusion
    :rtype:  ``set``
    """
    result = set()
    for key in filetree:
        # Recurse on directories
        if type(filetree[key]) == dict:
            result.update(expand_includes(path+'/'+key,filetree[key]))
        else:
            category = filetree[key]
            if category in ['all', 'android'] and not (os.path.splitext(key)[1] in SOURCE_EXT):
                result.add(path)
    return result


def place_project(config):
    """
    Places the Android project in the build directory
    
    :param config: The project configuration settings
    :type config:  ``dict``
    
    :return: The project directory
    :rtype:  ``str``
    """
    entries = ['root','build','camel','appid']
    util.check_config_keys(config,entries)
    
    if not '.' in config['appid']:
        raise ValueError('The value appid is missing an internal period: %s' % repr(config['appid']))
    
    # Create the build folder if necessary
    build = config['build']
    if not os.path.exists(build):
        os.mkdir(build)
    
    # Clear and create the temp folder
    build = util.remake_dir(build,MAKEDIR)
    
    # Copy the whole directory
    template = os.path.join(config['sdl2'],'templates','android','__project__')
    project  = os.path.join(build,config['camel'])
    shutil.copytree(template, project, copy_function = shutil.copy)
    
    # Move the main Java file
    java = os.path.join(project,'app','src','main','java')
    path = config['appid'].split('.')
    package = java
    for folder in path:
        package = os.path.join(package,folder)
        if not os.path.exists(package):
             os.mkdir(package)
    
    src = os.path.join(java,'__GAME__.java')
    dst = os.path.join(package,config['camel']+'.java')
    shutil.move(src, dst)
    
    return project


def determine_orientation(orientation):
    """
    Returns the Android orientation corresponding the config setting
    
    :param orientation: The orientation setting
    :type orientation:  ``str``
    
    :return: the Android orientation corresponding the config setting
    :rtype:  ``str``
    """
    if orientation == 'portrait':
        return 'portrait'
    elif orientation == 'landscape':
        return 'landscape'
    elif orientation == 'portrait-flipped':
        return 'reversePortrait'
    elif orientation == 'landscape-flipped':
        return 'reverseLandscape'
    elif orientation == 'portrait-either':
        return 'sensorPortrait'
    elif orientation == 'landscape-either':
        return 'sensorLandscape'
    elif orientation == 'multidirectional':
        return 'sensor'
    elif orientation == 'omnidirectional':
        return 'fullSensor'
    
    return 'unspecified'


def config_settings(config,project):
    """
    Configures all of the setting files in the Android project.
    
    These files include settings.gradle, (app) build.gradle, the AndroidManifest.xml
    file, and the custom Java class.
    
    :param config: The project configuration settings
    :type config:  ``dict``
    
    :param project: The project directory
    :type project:  ``str``
    """
    entries = ['camel','appid','assets','orientation','build_to_root']
    util.check_config_keys(config,entries)
    
    # Process settings.gradle
    settings = os.path.join(project,'settings.gradle')
    util.file_replace(settings,{'__project__':config['camel']})
    
    # Process build.gradle
    build = os.path.join(project,'app','build.gradle')
    assetdir = os.path.join('..','..','..',config['build_to_root'],config['assets'])
    assetdir = util.path_to_posix(assetdir)
    contents = {'__NAMESPACE__':config['appid'],'__ASSET_DIR__':assetdir}
    util.file_replace(build,contents)
    
    # Process AndroidManifest.xml
    manifest = os.path.join(project,'app','src','main','AndroidManifest.xml')
    contents = {'__GAME__':config['camel']}
    contents['__ORIENTATION__'] = determine_orientation(config['orientation'])
    util.file_replace(manifest,contents)
    
    # Process Java file
    package = os.path.join(project,'app','src','main','java',*(config['appid'].split('.')))
    java = os.path.join(package,config['camel']+'.java')
    contents = {'__GAME__':config['camel'],'__NAMESPACE__':config['appid']}
    util.file_replace(java,contents)
    
    # Process strings.xml
    strings = os.path.join(project,'app','src','main','res','values','strings.xml')
    util.file_replace(strings,{'__project__':config['name']})


def config_ndkmake(config,project):
    """
    Configures the Android.mk files
    
    :param config: The project configuration settings
    :type config:  ``dict``
    
    :param project: The project directory
    :type project:  ``str``
    """
    entries = ['sources','build_to_root','build_to_sdl2']
    util.check_config_keys(config,entries)
    
    # Find the folder offsets
    prefix = ['..','..','..','..']
    sdldir  = os.path.join(*prefix,config['build_to_sdl2'])
    sdldir  = util.path_to_posix(sdldir)
    srcdir  = os.path.join(*prefix,config['build_to_root'])
    srcdir  = util.path_to_posix(srcdir)
    
    # Modify the SDL2 Android.mk files recursively
    sdlroot  = os.path.join(project,'app','jni','sd2pckg')
    contents = {'__SDL2_PATH__':sdldir}
    util.directory_replace(sdlroot,contents,lambda path,file : file == 'Android.mk')
    
    # Modify the Project makefile
    srcmake = os.path.join(project,'app','jni','src','Android.mk')
    contents['__SDL2_PATH__'] = sdldir
    contents['__SOURCE_PATH__'] = srcdir
    
    # Source files
    filetree = config['source_tree']
    localdir = '$(LOCAL_PATH)'
    if len(config['source_tree']) == 1:
        key = list(config['source_tree'].keys())[0]
        localdir += '/'+util.path_to_posix(key)
        filetree = filetree[key]
    contents['__SOURCE_FILES__'] = expand_sources(localdir,filetree)
    
    # Include files
    inclist = []
    entries = config['include_dict']
    inclist.extend(entries['all'] if ('all' in entries and entries['all']) else [])
    inclist.extend(entries['android'] if ('android' in entries and entries['android']) else [])
    
    for item in expand_includes(localdir[len('$(LOCAL_PATH)/'):],filetree):
        inclist.append(item)
    
    incstr = ''
    for item in inclist:
        incstr += 'LOCAL_C_INCLUDES += $(PROJ_PATH)/'+util.path_to_posix(item)+'\n'
    contents['__EXTRA_INCLUDES__'] = incstr
    
    util.file_replace(srcmake,contents)


def config_cmake(config,project):
    """
    Configures the Android Cmake file
    
    :param config: The project configuration settings
    :type config:  ``dict``
    
    :param project: The project directory
    :type project:  ``str``
    """
    entries = ['sources','build_to_root','build_to_sdl2']
    util.check_config_keys(config,entries)
    cmake = os.path.join(project,'app','jni', 'CMakeLists.txt')
    prefix = ['..','..','..','..']
    
    context = {}
    context['__TARGET__'] = config['short']
    context['__APPNAME__'] = config['name']
    context['__VERSION__'] = config['version']
    
    # Set the SDL2 directory
    sdl2dir = os.path.join(*prefix,config['build_to_sdl2'])
    context['__SDL2DIR__'] = util.path_to_posix(sdl2dir)
    
    # Set the Asset directory
    assetdir = os.path.join(*prefix,config['build_to_root'],config['assets'])
    context['__ASSETDIR__'] = util.path_to_posix(assetdir)
    
    # Set the sources
    srclist = []
    entries = config['sources'][:]
    
    if 'cmake' in config and 'sources' in config['cmake']:
        if type(config['cmake']['sources']) == list:
            entries += config['cmake']['sources']
        elif config['cmake']['sources']:
            entries.append(config['cmake']['sources'])
    
    for item in entries:
        path = os.path.join(*prefix,config['build_to_root'],item)
        path = util.path_to_posix(path)
        srclist.append(path)
    
    context['__SOURCELIST__'] = '\n    '.join(srclist)
    
    # Set the include directories
    inclist = []
    entries = config['include_dict']
    inclist.extend(entries['all'] if ('all' in entries and entries['all']) else [])
    inclist.extend(entries['cmake'] if ('cmake' in entries and entries['cmake']) else [])
    
    incstr = ''
    for item in inclist:
        path = os.path.join(*prefix,config['build_to_root'],item)
        path = util.path_to_posix(path)
        incstr += 'list(APPEND EXTRA_INCLUDES "'+path+'")\n'
    context['__EXTRA_INCLUDES__'] = incstr
    
    util.file_replace(cmake,context)


def make(config):
    """
    Creates the Android Studio project
    
    This only creates the Android Studio project; it does not actually build the project.
    To build the project, you must open it up in Android Studio.
    
    :param config: The project configuration settings
    :type config:  ``dict``
    """
    print()
    print('Configuring Android build files')
    print('-- Copying Android Studio project')
    project = place_project(config)
    print('-- Modifying gradle settings')
    config_settings(config,project)
    print('-- Modifying project makefiles')
    config_ndkmake(config,project)
    config_cmake(config,project)
    
    if 'icon' in config:
        print('-- Generating icons')
        res = os.path.join(project,'app','src','main')
        config['icon'].gen_android(res)
