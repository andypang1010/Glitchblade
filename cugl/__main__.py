"""
Python Script for CUGL Projects

After years of sponsoring student projects, we got tired of supporting all the 
different IDE projects. Particularly because they were so brittle and students 
refused to read the directions on how to change them properly.

Originally, we had hoped CMake would solve the problem. However, CMake has a
lot of a problems that make it unsuitable for the mobile builds by itself. 
All mobile builds require significant configuration with respect to asset 
bundles, app identifiers, and code signing. For example, while Android does
use CMake, that is only a small part of the Android Studio project that includes
Java files, resources, and manifests. In our experience, students would update 
the CMake file but ignore the other files. In cases where they built off of 
demo code, this meant that their applications conflicted with each other and 
could not be submitted to the Google Store.

The situation for iOS is a little better, as CMake toolchains can generate a
complete XCode project. But these projects are extremely brittle with absolute
path names everywhere. The result is an XCode project that is fine for a solo
developer, but inappropriate for a team.

Ultimately, we concluded that while CMake is mostly fine on desktop platforms
(and is the ideal build system for Linux), it could not be our primary build
system. This Python script is the compromise. For a given CUGL project, we 
generate the  project files for each target platform. It is designed so that 
users can safely  edit code without regenerating the project files. Furthermore,
these project files use relative path names, making them repository safe
(with the exception of CUGL extensions, which uses CMake to make platform
specific subprojects).

With that said, major changes to the  project (even something as simple as 
making a new icon) do require a rebuild of the project files. We generally
recommend the students to rebuild the projects before each sprint submission.

To work properly, the CUGL project must have a config.yml in its root directory. 
See the documentation for the format of this YML file.

Author:  Walker M. White
Version: 7/10/24
"""
import os, os.path
import argparse
import re
import scripts.iconify as iconify
import scripts.util as util

# The support platforms
PLATFORMS = { 'android' : 'Android build via Android Studio',
              'apple' : 'Both macOS and iOS build via XCode',
              'cmake' : 'Experimental Linux build via CMake',
              'macos' : 'macOS only build via XCode',
              'ios' : 'iOS only build via XCode',
              'windows' : 'Windows build via Visual Studio'
            }


# The allowed orientations
ORIENTATIONS = { 'portrait' : 'portrait mode only, home at the bottom (notch at top)',
                 'portrait-flipped' : 'portrait mode only, home at the top (notch at bottom)',
                 'portrait-either' : 'portrait mode only, but with direction depending on device orientiation',
                 'landscape' : 'landscape mode only, home at the right (notch at left)',
                 'landscape-flipped' : 'landscape mode only, home at the left (notch at right)',
                 'landscape-either' : 'landscape mode only, but with direction depending on device orientiation',
                 'multidirectional' : 'either portrait or landsscape mode, but not flipped',
                 'omindirectional' : 'all possible modes are allowed, depending on device orientiation'
                 }


def setup():
    """
    Returns an initialized parser for this script.
    
    :return: The parsed args
    :rtype:  ``Namespace``
    """
    parser = argparse.ArgumentParser(description='Build an SDL2 project.')
    parser.add_argument('project', type=str, help='The project directory')
    parser.add_argument('-b', '--build', type=str, help='The build directory')
    parser.add_argument('-c', '--config', type=str, help='The configuration file')
    parser.add_argument('-t', '--target', type=str,
                        choices=['android','apple','macos','ios','windows','cmake'],
                        help='The active build target')
    return parser.parse_args()


def get_config(args):
    """
    Returns the config settings for the given project
    
    Any CUGL project must have a config.yml in the root directory in order for the
    build system to work properly. This method parses that YML file and converts it
    into a Python data structure. If there is no config.yml, this method returns None.
    
    :param target: The path to the CUGL project
    :type data:  ``str``
    
    :return: a Python representation of the YML file
    :rtype:  ``dict``
    """
    import yaml
    
    if not args.config is None:
        config = args.config
    else:
        config = os.path.join(args.project,'config.yml')
    if not os.path.exists(config):
        raise FileNotFoundError('File %s does not exist.' % repr(config))
    
    data = None
    with open(config, encoding = 'utf-8') as file:
        data = file.read()
        data = yaml.load(data,Loader=yaml.Loader)
    
    if not data is None:
        data['uuids'] = util.UUIDService()
        if util.entry_has_path(data,'suffix') and data['suffix']:
            if type(data['suffix']) == bool:
                uuid = data['uuids'].getAppleUUID(os.path.abspath(config))
                uuid = data['uuids'].applyPrefix('G',uuid)[:8]
                data['suffix'] = uuid
                util.file_replace_after(config,{'suffix:':' '+uuid})
            data['appid'] += '.'+data['suffix']
        
        # Now add computed attributes
        data['root'] = os.path.abspath(args.project)
        data['cugl'] = os.path.split(__file__)[0]
        
        if not args.build is None:
            data['build'] = os.path.abspath(args.build)
        else:
            data['build'] = os.path.normpath(os.path.join(data['root'],data['build']))
        
        data['project'] = args.project
        data['build_to_cugl'] = os.path.relpath(data['cugl'],data['build'])
        data['build_to_root'] = os.path.relpath(data['root'],data['build'])
        data['build_to_project'] = os.path.relpath(args.project,data['build'])
    
    return data


def snake2Camel(text):
    """
    Returns a (capitalized) version of text in camel case
    
    All underscores are removed, and capital letters are placed at underscore boundaries.
    
    https://www.geeksforgeeks.org/python-convert-snake-case-string-to-camel-case/
    
    :param text: The text in snake case to convert
    :type text:  ``str``
    
    :return: a (capitalized) version of text in camel case
    :rtype:  ``str``
    """
    temp = text.split('_')
    return ''.join(ele[:1].upper()+ele[1:] for ele in temp)


def normalize(config,args):
    """
    Normalizes the config file to contain missing data
    
    :param config: The project configuration settings
    :type config:  ``dict``
    """
    if not 'name' in config:
        config['name'] = 'Game'
    if not 'version' in config:
        config['version'] = '1.0'
    
    pattern = re.compile(r'[^\w_]+')
    if not 'short' in config:
        config['short'] = pattern.sub('_',config['name'])
    elif not config['short'].isidentifier():
        config['short'] = pattern.sub('_',config['short'])
    
    camel = snake2Camel(config['short'])
    if camel == '':
        camel = 'Game'
    elif not camel[0].isalpha():
        camel = 'G'+camel
    config['camel'] = camel
    
    # Add other missing information
    if not 'orientation' in config:
        config['orientation'] = 'landscape'
    else:
        config['orientation'] = config['orientation'].lower()
    
    if not 'headless' in  config:
        config['headless'] = False
        
    if not 'modules' in config or type(config['modules']) != list:
        config['modules'] = 'all'
    
    if args.target:
        config['targets'] = [args.target]
    elif 'targets' in config:
        if type(config['targets']) == list:
            config['targets'] = list(map(lambda x: x.lower(), config['targets']))
        elif type(config['targets']) == str:
            config['targets'] = [ config['targets'].lower() ]
        else:
            config['targets'] = []
    else:
        config['targets'] = []
    
    # We only allow one of these
    if 'apple' in config['targets']:
        if 'ios' in config['targets']:
            config['targets'].remove('ios')
        if 'macos' in config['targets']:
            config['targets'].remove('macos')
    elif 'macos' in config['targets'] and 'ios' in config['targets']:
        config['targets'].remove('ios')
        config['targets'].remove('macos')
        config['targets'].append('apple')
    
    if not 'assets' in config:
        config['assets'] = 'assets'


def validate(config):
    """
    Returns False (with an error message) if the config file properties are wrong
    
    :param config: The project configuration settings
    :type config:  ``dict``
    
    :return: False (with an error message) if the config file properties are wrong
    :rtype:  ``bool``
    """
    if not 'appid' in config:
        print('ERROR: Configuration file is missing the appid')
        return False
    elif not '.' in config['appid']:
        print('ERROR: appid value must have at least one period: %s' % repr(config['appid']))
        return False
    
    if not 'sources' in config or not config['sources']:
        print('ERROR: Configuration file has no source files')
        return False
    
    if 'orientation' in config:
        if not config['orientation'] in ORIENTATIONS:
            print('ERROR: %s is not a valid orientiation' % repr(config['orientation']))
            print('Choose one of the following: ')
            pad = max(map(lambda x : len(x), ORIENTATIONS))+2
            for key in ORIENTATIONS:
                prompt = (key+': ')
                prompt += ' '*(pad-len(prompt))
                print('- %s %s' % (prompt,ORIENTATIONS[key]))
            return False
    
    if 'targets' in config and config['targets']:
        for target in config['targets']:
            if not target in PLATFORMS:
                print('ERROR: %s is not a valid platform' % repr(target))
                print('Choose one of the following: ')
                pad = max(map(lambda x : len(x), PLATFORMS))+2
                for key in PLATFORMS:
                    prompt = (key+': ')
                    prompt += ' '*(pad-len(prompt))
                    print('- %s %s' % (prompt,PLATFORMS[key]))
                return False
    
    return True


def expand_modules(config):
    """
    Updates the config to identify the modules to link
    
    As CUGL grows, our build times have gotten larger an larger. While we still
    believe in a source code release, breaking it up into modules allows us to
    ignore unnecessary code.

    :param config: The project configuration settings
    :type config:  ``dict``
    """
    modules = ['graphics','audio', 'scene2','scene3','physics2','netcode','physics2::distrib']
    modset = {}
    if config['modules'] == 'all' or 'all' in config['modules']:
        config['modules'] = dict()
        for item in modules:
            config['modules'][item] = True
    else:
        actives = config['modules']
        config['modules'] = dict()
        for item in modules:
             config['modules'][item] = item in actives
    
    # Handle prereqs
    if config['modules']['scene2'] or config['modules']['scene3']:
        config['graphics'] = True
    
    if not config['headless']:
        config['modules']['graphics'] = True
    
    if config['modules']['physics2::distrib']:
        config['modules']['physics2'] = True
        config['modules']['netcode'] = True


def verify_externals(config):
    """
    Quickly verifies that each external is a folder with a CMakeLists.txt
    
    :param config: The project configuration settings
    :type config:  ``dict``
    """
    if not 'external' in config:
        return
    
    invalid = []
    for pos in range(len(config['external'])):
        item = config['external'][pos]
        if not 'name' in item:
            print('ERROR: Unnamed external package at position %d' % pos)
            invalid.append(pos)
            continue
        name = item['name']
        if not 'path' in item:
            print('ERROR: Cannot locate package %s' % repr(name))
            invalid.append(pos)
            continue
        path = item['path']
        location = os.path.join(config['project'],path)
        if not os.path.exists(location):
            print('ERROR: Cannot locate package %s' % repr(name))
            invalid.append(pos)
            continue
        location = os.path.join(location,'CMakeLists.txt')
        if not os.path.exists(location):
            print("ERROR: Cannot package %s does not contain a 'CMakeLists.txt'" % repr(name))
            invalid.append(pos)
            continue
        # Normalize targets if it is okay
        if not 'targets' in item:
            item['targets'] = []
        elif not type(item['targets']) == list:
            item['targets'] = [item['targets']]
    
    if len(invalid) > 0:
        actual = []
        for pos in range(len(config['external'])):
            if not pos in invalid:
                actual.append(config['external'][pos])
        config['external'] = actual


def expand_sources(config):
    """
    Updates the config to include all of the project sources
    
    The sources are placed into the entry 'source_tree' as a filetree. That is
    a recursive dictionary where each each key is a path relative to parent (top
    level keys are relative to the project root). The leaves are two-element
    tuples were the first element is the file and the second is a target. The
    target 'all' is for all targets.
    
    :param config: The project configuration settings
    :type config:  ``dict``
    """
    root = config['root']
    
    filetree = {}
    # Get the main sources
    for item in config['sources']:
        files = util.expand_wildcards(root,item)
        util.insert_filetree(filetree,files,'all')
    
    # Get the auxiliary sources
    for target in ['android','apple','macos','ios','windows','cmake']:
        if target in config and 'sources' in config[target]:
            if type(config[target]['sources']) == list:
                for item in config[target]['sources']:
                    files = util.expand_wildcards(root,item)
                    util.insert_filetree(filetree,files,target)
            elif config[target]['sources']:
                files = util.expand_wildcards(root,config[target]['sources'])
                util.insert_filetree(filetree,files,target)
    
    config['source_tree'] = filetree


def expand_assets(config):
    """
    Updates the config to include all of the assets
    
    The assets are placed into the entry 'asset_list' as a list. The elements are
    two-element tuples were the first element is the file and the second is an
    indication of 'file' or 'directory'.
    
    :param config: The project configuration settings
    :type config:  ``dict``
    """
    assetlst = []
    assetdir = config['assets'].split('/')
    if len(assetdir) > 0 and assetdir[0] == '':
        assetdir = assetdir[1:]
    
    root = config['root']
    path = os.path.join(root,*assetdir)
    if os.path.isdir(path):
        for item in os.listdir(path):
            if item[0] != '.':
                if os.path.isdir(os.path.join(path,item)):
                    assetlst.append((item,'directory'))
                else:
                    assetlst.append((item,'file'))
    
    config['asset_list'] = assetlst


def expand_includes(config):
    """
    Updates the config to include all of the include directories
    
    Include directories are specified as a list of paths (their organization
    is IDE dependent) The directories are placed into the entry 'include_dict' 
    as a dict. The keys are the targets.

    :param config: The project configuration settings
    :type config:  ``dict``
    """
    root = util.posix_to_path(config['root'])
    keyword = 'includes'
    
    result = {}
    includes = []
    if keyword in config and config[keyword]:
        if type(config[keyword]) == list:
            for item in config[keyword]:
                files = util.expand_wildcards(root,item,'directory')
                includes.extend(map(util.posix_to_path,files))
        elif config[keyword]:
            files = util.expand_wildcards(root,config[keyword],'directory')
            includes.extend(map(util.posix_to_path,files))
        result['all'] = includes
    
    # Get the auxiliary sources
    for target in ['android','apple','macos','ios','windows','cmake']:
        includes = []
        if target in config and keyword in config[target]:
            if type(config[target][keyword]) == list:
                for item in config[target][keyword]:
                    files = util.expand_wildcards(root,item,'directory')
                    includes.extend(map(util.posix_to_path,files))
            elif config[target][keyword]:
                files = util.expand_wildcards(root,config[target][keyword],'directory')
                includes.extend(map(util.posix_to_path,files))
        result[target] = includes
    
    # And any external includes
    if 'external' in config:
        includes = []
        for item in config['external']:
            if keyword in item:
                if type(item[keyword]) == list:
                    for elt in item[keyword]:
                        files = util.expand_wildcards(root,elt,'directory')
                        includes.extend(map(util.posix_to_path,files))
                elif item[keyword]:
                    files = util.expand_wildcards(root,item[keyword],'directory')
                    includes.extend(map(util.posix_to_path,files))
        if 'all' in result:
            result['all'].extend(includes)
        else:
            result['all'] = includes
    
    config['include_dict'] = result


def gather_defines(defset):
    """
    Returns a definition dictionary from the given definition set
    
    A definition set can be a single value (usually a string), a list, or 
    a dict.  In the case that it is a dict, the keywords are all build types
    (e.g. 'all', 'debug', 'release'). This function converts the definition
    set into a definition dictionary that is keyed on build types. Any
    definition that is not associated with a build type is assigned to 
    build type 'all'
    
    :param defset: The set of definitions
    :type defset:  ``dict``, ``list``, or ``str``
    
    :return: A definition dictionary normalizing the values
    :rtype:  ``dict``
    """
    result = {}
    if type(defset) in [list,tuple]:
        defines = []
        for item in defset:
            defines.append(item)
        result['all'] = defines
    
    elif type(defset) == dict:
    
        for subkey in defset:
            subset = defset[subkey]
            if type(subset) in [list,tuple]:
                defines = []
                for item in config[keyword]:
                    defines.append(item)
                result[subkey] = defines
            elif subset:
                result[subkey] = [ subset ]
    
    elif defset:
        result['all'] = [ defset ]
    
    return result


def expand_defines(config):
    """
    Updates the config to include all of the preprocessor definitions

    Preprocessor definitions are specified as list of definitions without the
    -D modifier (their execution is IDE dependent). The defines are placed into 
    the entry 'define_dict' as a dict. The keys are the targets.

    :param config: The project configuration settings
    :type config:  ``dict``
    """
    keyword = 'defines'
    subkeys = ['all','debug','release']

    result = {}
    if keyword in config and config[keyword]:
        result['all'] = gather_defines(config[keyword])
    
    # Get the auxiliary sources
    for target in ['android','apple','macos','ios','windows','cmake']:
        if target in config and keyword in config[target]:
            result[target] = gather_defines(config[target][keyword])
    
    # And any external defines
    if 'external' in config:
        for item in config['external']:
            if keyword in item:
                defines = gather_defines(item[keyword])
                if 'all' in result:
                    result['all'] = util.merge_dict(result['all'],defines)
                else:
                     result['all'] = defines
    
    config['define_dict'] = result


def build_icon(config):
    """
    Creates an icon maker if icon information is present
    
    :param config: The project configuration settings
    :type config:  ``dict``
    """
    if not 'icon' in config:
        return
    
    if type(config['icon']) == str:
        foreground = config['icon']
        background = 'ffffff'
        transparent = False
        rounded = True
    elif type(config['icon']) == dict:
        foreground = None if not 'image' in config['icon'] else config['icon']['image']
        background = '#fffff' if not 'background' in config['icon'] else config['icon']['background']
        transparent = 'transparent' in config['icon'] and config['icon']['transparent']
        rounded = not 'rounded' in config['icon'] or config['icon']['rounded']
    
    if foreground:
        expand = util.posix_split(foreground)
        if not os.path.ismount(expand[0]):
            foreground = os.path.join(config['root'],*expand)
        config['icon'] = iconify.IconMaker(foreground,background,transparent,rounded)


def main():
    """
    Runs the build script
    """
    args = setup()
    config = get_config(args)
    normalize(config,args)
    if not validate(config) or not config['targets']:
        return

    print('Configuring modules')
    expand_modules(config)
    verify_externals(config)
    
    print('Locating assets and source files')
    expand_sources(config)
    expand_assets(config)
    expand_includes(config)
    expand_defines(config)
    build_icon(config)
    
    for target in config['targets']:
        if target == 'cmake':
            import scripts.cmake
            scripts.cmake.make(config)
        if target == 'android':
            import scripts.android
            scripts.android.make(config)
        if target in ['apple','macos','ios']:
            import scripts.apple
            scripts.apple.make(config)
        if target == 'windows':
            import scripts.windows
            scripts.windows.make(config)


if __name__ == '__main__':
    main()