"""
Python Script for CMake Builds

While CMake is naturally "out of source", there are still many reasons why we want to
autogenerate the CMake files. In particular, it makes it easier to link in a new SDL2
base (as upgrades are released) without having to change any files in the project.

Note that, because of how CMake works, this script both autogenerates a new CMake file
and then makes a separate build directory. For cleanliness, the autogenerated CMake
file is a subdirectory of the build directory.


Author: Walker M. White
Date:   11/29/22
"""
import os, os.path
import shutil
import subprocess
import re, string
from . import util

# The subbuild folder for cmake
MAKEDIR = 'cmake'
# The subfolder for containing the results

def place_project(config):
    """
    Places the CMakeLists.txt in the project directory

    :param config: The project configuration settings
    :type config:  ``dict``

    :return: The project directory
    :rtype:  ``str``
    """
    entries = ['short','sdl2','build']
    util.check_config_keys(config,entries)

    # Create the build folder if necessary
    build = config['build']
    if not os.path.exists(build):
        os.mkdir(build)

    # Clear and create the build folder
    project = util.remake_dir(build,MAKEDIR)
    src  = os.path.join(config['sdl2'],'templates','cmake','ReadMe.md')
    dst = os.path.join(project, 'ReadMe.md')
    shutil.copyfile(src,dst)

    # Copy the CMakefile
    src = os.path.join(config['sdl2'],'templates','cmake','CMakeLists.txt')
    dst = os.path.join(project, 'CMakeLists.txt')
    shutil.copyfile(src,dst)

    # Copy the flatpak directory
    appid = config['appid']
    if 'suffix' in config and config['suffix']:
        appid = '.'.join(appid.split('.')[:-1])

    src = os.path.join(config['sdl2'],'templates','cmake','flatpak')
    dst = os.path.join(project, 'flatpak')
    shutil.copytree(src, dst, copy_function = shutil.copy)

    ymlsrc = os.path.join(dst,'__APP_ID__.yml')
    ymldst = os.path.join(dst,appid+'.yml')
    shutil.move(ymlsrc,ymldst)

    # Make the work folder
    workdir = os.path.join(project,'cmake')
    os.mkdir(workdir)
    return project


def config_cmake(config,project):
    """
    Configures the contents of CMakeLists.txt

    The CMakeLists.txt list template must be modified to use the values in the
    config file. This function reads the CMakeLists.txt into a string, modifies
    the string, and then writes out the result.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param project: The project directory
    :type project:  ``str``
    """
    entries = ['root','name','short','version','sdl2','sources', 'assets', 'build_to_sdl2','build_to_root']
    util.check_config_keys(config,entries)

    cmake = os.path.join(project, 'CMakeLists.txt')

    # Set the application name and version
    context ={}
    context['__TARGET__'] = config['short']
    context['__APPNAME__'] = config['name']
    context['__VERSION__'] = config['version']

    # Set the SDL2 directory
    prefix = ['..']
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


def config_flatpak(config,project):
    """
    Configures the contents of the flatpak directory.

    The Flatpak scripts must all be modified to use the unique id for this
    application.  This will read each of the three files and then Modify
    them.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param project: The project directory
    :type project:  ``str``
    """
    entries = ['root','name','short','appid']
    util.check_config_keys(config,entries)

    appid = config['appid']
    if 'suffix' in config and config['suffix']:
        appid = '.'.join(appid.split('.')[:-1])

    short = config['short'].lower()

    context = {'__APP_ID__':appid, '__SHORT__':short}
    ymlfile = os.path.join(project, 'flatpak', appid+'.yml')
    util.file_replace(ymlfile,context)

    name = config['name']
    pattern = re.compile(r'[^\w_]+')
    shortcut = pattern.sub('',name)
    context['__GAME__'] = name
    context['__SHORTCUT__'] = shortcut

    shellfile = os.path.join(project, 'flatpak', 'build.sh')
    util.file_replace(shellfile,context)

    flatfile = os.path.join(project, 'flatpak', 'flatpak-run.sh')
    util.file_replace(flatfile,context)


def make(config):
    """
    Creates the CMake build

    This only creates the CMake build; it does not actually build the project. To build
    the project, go to the CMake build directory and type:

        cmake --build .

    :param config: The project configuration settings
    :type config:  ``dict``
    """
    print()
    print('Configuring CMake files')
    print('-- Creating the build directory')
    project = place_project(config)
    print('-- Configuring top level CMakeLists.txt')
    config_cmake(config,project)
    print('-- Configuring Flatpak settings')
    config_flatpak(config,project)
