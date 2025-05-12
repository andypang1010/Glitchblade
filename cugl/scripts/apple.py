"""
Python Script for Apple Builds

Another reason for the custom build set-up for CUGL is iOS builds. There is a
lot to set up in an iOS build beyond the source code. Note, however, that we do
not separate iOS and macOS projects. We build one XCode project and put it in
the Apple build folder.

This code is some of the most complicated of our build script namely due to the
fact that Xcode projects do not use a standardized file format for their data.
Instead of xml/json, it is an eclectic file format that freely mixes () and {}
to group subelements, and does so with no apparent explanation. As a result,
a lot of the file manipulation is essentially hard-coded.

This is made worse by our latest extension allowing students to incorporate
third-party libraries. In this code we have to integrate two Xcode projects
together. Because of the high levels of indirection in these project files
(references of proxies of containers), this requires a significant amount of
code to process.

Author:  Walker M. White
Version: 7/10/24
"""
import os, os.path
import subprocess
import shutil
import glob
from . import util

# To indicate the type of an XCode entry
TYPE_APPLE = 0
TYPE_MACOS = 1
TYPE_IOS   = 2


# These are unique to the template project
MAC_TARGET = 'EB0F3C9527FB9DCB0037CC66'
IOS_TARGET = 'EBC7AEC127FBB41F001F1467'
ALL_TARGET = 'EB0F3C8E27FB9DCB0037CC66'

# The subbuild folder for the project
MAKEDIR = 'apple'

# Supported header extensions
HEADER_EXT = ['.h', '.hh', '.hpp', '.hxx', '.hm', '.inl', '.inc']
# Supportes source extensions
SOURCE_EXT = ['.cpp', '.c', '.cc', '.cxx', '.m', '.mm','.asm', '.asmx','.swift']


def determine_orientation(orientation):
    """
    Returns the Apple orientation corresponding the config setting

    :param orientation: The orientation setting
    :type orientation:  ``str``

    :return: the Apple orientation corresponding the config setting
    :rtype:  ``str``
    """
    if orientation == 'portrait':
        return 'UIInterfaceOrientationPortrait'
    elif orientation == 'landscape':
        return 'UIInterfaceOrientationLandscapeRight'
    elif orientation == 'portrait-flipped':
        return 'UIInterfaceOrientationPortraitUpsideDown'
    elif orientation == 'landscape-flipped':
        return 'UIInterfaceOrientationLandscapeLeft'
    elif orientation == 'portrait-either':
        return '"UIInterfaceOrientationPortrait UIInterfaceOrientationPortraitUpsideDown"'
    elif orientation == 'landscape-either':
        return '"UIInterfaceOrientationLandscapeRight UIInterfaceOrientationLandscapeLeft"'
    elif orientation == 'multidirectional':
        return '"UIInterfaceOrientationPortrait UIInterfaceOrientationLandscapeRight"'
    elif orientation == 'omnidirectional':
        return '"UIInterfaceOrientationPortrait UIInterfaceOrientationLandscapeRight UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationPortraitUpsideDown"'

    return 'UIInterfaceOrientationLandscapeRight'


def build_groups(path,filetree,uuidsvc):
    """
    Creates the groups for the XCode project

    Groups are virtual directories in XCode. They must each have a unique UUID
    and be connected in a tree-like manner. This function returns a dictionary
    representing this tree.  The keys are the unique UUIDS and the values are
    three-element tuples representing each child.  These tuples contain the
    child UUID, the child path, and an annotation.  If the path is a leaf in
    the filetree, the annotation is the same as the filetree. Otherwise it is
    simply the tag 'group'.

    :param path: The path to the root of the filetree in POSIX form
    :type config:  ``str``

    :param filetree: The filetree of paths
    :type filetree:  ``dict``

    :param uuidsvc: The UUID generator
    :type uuidsvc:  ``UUIDService``

    :return: The dictionary of the groups together with the UUID of the root
    :rtype:  (``str``,``dict``)
    """
    uuid = uuidsvc.getAppleUUID('GROUP://'+path)
    uuid = uuidsvc.applyPrefix('CD',uuid)
    contents = []
    result = {uuid:('',contents)}
    for item in filetree:
        if type(filetree[item]) == dict: # This is a group
            subresult = build_groups(path+'/'+item,filetree[item],uuidsvc)
            contents.append((subresult[0],item,'group'))
            for key in subresult[1]:
                if key == subresult[0]:
                    result[key] = (item,subresult[1][key][1])
                else:
                    result[key] = subresult[1][key]
        else:
            newid = uuidsvc.getAppleUUID('FILE://'+path+'/'+item)
            newid = uuidsvc.applyPrefix('BA',newid)
            contents.append((newid,item,filetree[item]))

    return (uuid,result)


def make_directory(config):
    """
    This creates the build directory for Apple projects

    This also copies over a private copy of the CUGL Xcode so we can do some
    optimizations.

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

    # Create a CUGL subfolder if necessary
    cugldir = os.path.join(build,'cugl')
    if not os.path.exists(cugldir):
        os.mkdir(cugldir)

    # Copy the CUGL project
    src = os.path.join(config['cugl'],'buildfiles',MAKEDIR,'cugl.xcodeproj')
    dst = os.path.join(cugldir,'cugl.xcodeproj')
    shutil.copytree(src, dst, copy_function = shutil.copy)

    # Fix the references in this file
    build_to_cugl = '../../'+config['build_to_cugl']
    contents = {'../..':build_to_cugl}
    contents['support/'] = build_to_cugl+'/buildfiles/apple/support/'
    if config['headless']:
        contents['CU_USE_OPENGL'] = 'CU_HEADLESS'

    source = os.path.join(build,'cugl','cugl.xcodeproj','project.pbxproj')
    util.file_replace(source,contents)

    # And remove the final schemes
    source = os.path.join(build,'cugl','cugl.xcodeproj','xcshareddata')
    shutil.rmtree(source,ignore_errors=True)


def place_project(config):
    """
    Places the XCode project in the build directory

    :param config: The project configuration settings
    :type config:  ``dict``

    :return: The project directory
    :rtype:  ``str``
    """
    entries = ['root','build','camel']
    util.check_config_keys(config,entries)

    # Create the build folder if necessary
    build = os.path.join(config['build'],MAKEDIR)
    if not os.path.exists(build):
        os.mkdir(build)

    # Copy the XCode project
    template = os.path.join(config['cugl'],'templates',MAKEDIR,'app.xcodeproj')
    project  = os.path.join(build,config['camel']+'.xcodeproj')
    shutil.copytree(template, project, copy_function = shutil.copy)

    # Now copy the resources folder
    src = os.path.join(config['cugl'],'templates',MAKEDIR,'Resources')
    dst  = os.path.join(build,'Resources')
    shutil.copytree(src, dst, copy_function = shutil.copy)

    return project


def parse_pbxproj(project):
    """
    Returns a dictionary composed of XCode objects

    An XCode object is a single XCode entity representing a feature like a file,
    a group, or a build setting. When we modify an XCode file, we do it by adding
    and removing text at the object level. Parsing the XCode file into its
    component objects makes it easier to modify.

    The resulting value is a dictionary that maps XCode PBX entries (represented
    as strings) to lists of objects (which are also all strings).

    :param project: The Apple build directory
    :type project:  ``str``

    :return: The dictionary of XCode objects
    :rtype:  ``dict``
    """
    source = os.path.join(project,'project.pbxproj')
    with open(source) as file:
        state = 'PBXHeader'
        block = []
        accum = None
        brace = 0
        contents = {'PBXOrder':[]}
        contents[state] = block
        contents['PBXOrder'].append(state)
        between  = False

        check = 0
        for line in file:
            check += 1
            advance  = False
            if state != 'PBXFooter':
                advance = '/* Begin' in line and 'section */' in line
                if advance:
                    pos = line.find('/* Begin ')+len('/* Begin ')
                    state = line[pos:]
                    pos = state.find('section */')
                    state = state[:pos].strip()
                    contents['PBXOrder'].append(state)
            complete = ('/* End '+state+' section */') in line

            if advance:
                # Time to advance state
                if accum:
                    block.append(accum)
                block = []
                accum = None
                brace = 0
                contents[state] = block
                between = False
            elif state in ['PBXHeader','PBXFooter']:
                accum = line if accum is None else accum+line
            elif complete:
                if accum and accum != '\n':
                    block.append(accum)
                between = True
            elif between:
                # Look for evidence of the end
                if len(line.strip()) > 0:
                    state = 'PBXFooter'
                    contents['PBXOrder'].append(state)
                    if accum:
                        block.append(accum)
                    block = []
                    accum = line
                    brace = 0
                    contents[state] = block
                    between = False
            else:
                # Now it is time to parse objects
                brace += util.group_parity(line,'{}')
                if brace < 0:
                    print('ERROR: Braces mismatch at line %d of pbxproj file' % check)
                    return None
                else:
                    accum = line if accum is None else accum+line
                    if brace == 0:
                        if accum != '\n':
                            block.append(accum)
                        accum = None

        # Do not forget the footer
        block.append(accum)

    return contents


def place_entries(obj,entries,prefix='files'):
    """
    Returns a copy of XCode object with the given entries added

    An XCode object is one recognized by parse_pbxproj. Entries are typically written
    between two parentheses, with a parent value such as 'files' or 'children'. This
    function adds these entries to the parentheses, properly indented (so the entries
    should not be indented beforehand).

    :param obj: The XCode object to modify
    :type obj:  ``str``

    :param entries: The entries to add
    :type entries:  ``list`` of ``str``

    :param prefix: The parent annotation for the child list
    :type prefix:  ``str``

    :return: the object with the given entries added
    :rtype:  ``str``
    """
    indent = '\n\t\t\t\t'
    line = obj.find(prefix+' = (')
    if line == -1:
        return
    pos = obj.find('\n',line)
    children = indent+indent.join(entries)
    return obj[:pos]+children+obj[pos:]


def reassign_pbxproj(config,pbxproj):
    """
    Modifies pbxproj to reflect the current build location

    The template XCode project has several variables that begin and end with
    double underscores. The vast  majority of these are directories that must be
    updated to match the current location of the build directory. This function
    updates many of these variables

    :param config: The project configuration settings
    :type config:  ``dict``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``
    """
    # SDL directory (relative)
    cugldir  = '../'+util.path_to_posix(config['build_to_cugl'])
    rootdir  = '../'+util.path_to_posix(config['build_to_root'])
    sdl2dir  = '../'+util.path_to_posix(os.path.join(config['build_to_cugl'],'sdlapp'))
    poly2dir = '../'+util.path_to_posix(os.path.join(config['build_to_cugl'],'external','poly2tri'))
    box2dir  = '../'+util.path_to_posix(os.path.join(config['build_to_cugl'],'external','box2d'))
    netdir   = '../'+util.path_to_posix(os.path.join(config['build_to_cugl'],'external','libdatachannel'))

    # Replace the project path
    section  = pbxproj['PBXFileReference']
    licenses = '../'+cugldir+"/licenses"
    index = -1
    for pos in range(len(section)):
        if 'cugl.xcodeproj' in section[pos]:
            entry = section[pos]
            pos0 = entry.find('path')
            pos1 = entry.find(';',pos0)
            section[pos] = entry[:pos0]+'path = cugl/cugl.xcodeproj'+entry[pos1:]
        if '__LICENSES__' in section[pos]:
            section[pos] = section[pos].replace('__LICENSES__','"'+licenses+'"')

    # Asset and Source directory
    section = pbxproj['PBXGroup']
    assetdir  = rootdir+'/'+util.path_to_posix(config['assets'])
    sourcedir = rootdir

    if len(config['source_tree']) == 1:
        sourcedir += '/'+util.path_to_posix(list(config['source_tree'].keys())[0])

    for pos in range(len(section)):
        if '__ASSET_DIR__' in section[pos]:
            section[pos] = section[pos].replace('__ASSET_DIR__','"'+assetdir+'"')
        if '__SOURCE_DIR__' in section[pos]:
            section[pos] = section[pos].replace('__SOURCE_DIR__','"'+sourcedir+'"')

    # Expand the remaining build settings (display name, target id)
    section = pbxproj['XCBuildConfiguration']
    appid = config['appid']
    macid = appid.split('.')
    macid = '.'.join(macid[:-1]+['mac']+macid[-1:])
    iosid = appid.split('.')
    iosid = '.'.join(iosid[:-1]+['ios']+iosid[-1:])

    # Handle all includes except the customs (which wait on the modules)
    for pos in range(len(section)):
        if '__CUGL_INCLUDE__' in section[pos]:
            section[pos] = section[pos].replace('__CUGL_INCLUDE__','"$(SRCROOT)/'+cugldir+'/include"')
        if '__POLY2_INCLUDE__' in section[pos]:
            section[pos] = section[pos].replace('__POLY2_INCLUDE__','"$(SRCROOT)/'+poly2dir+'"')
        if '__BOX2D_INCLUDE__' in section[pos]:
            section[pos] = section[pos].replace('__BOX2D_INCLUDE__','"$(SRCROOT)/'+box2dir+'/include"')
        if '__DATACH_INCLUDE__' in section[pos]:
            section[pos] = section[pos].replace('__DATACH_INCLUDE__','"$(SRCROOT)/'+netdir+'/include"')
        if '__SDL_INCLUDE__' in section[pos]:
            section[pos] = section[pos].replace('__SDL_INCLUDE__','"$(SRCROOT)/'+sdl2dir+'/include"')
        if '__MAC_APP_ID__' in section[pos]:
            section[pos] = section[pos].replace('__MAC_APP_ID__',macid)
        if '__IOS_APP_ID__' in section[pos]:
            section[pos] = section[pos].replace('__IOS_APP_ID__',iosid)

    # Update the targets
    for category in ['PBXProject','XCConfigurationList','PBXNativeTarget']:
        section = pbxproj[category]
        for pos in range(len(section)):
            if 'app-mac' in section[pos]:
                section[pos] = section[pos].replace('app-mac',config['short'].lower()+'-mac')
            if 'app-ios' in section[pos]:
                section[pos] = section[pos].replace('app-ios',config['short'].lower()+'-ios')

    # Finally, update the display name everywhere
    for category in pbxproj:
        section = pbxproj[category]
        for pos in range(len(section)):
            if '__DISPLAY_NAME__' in section[pos]:
                section[pos] = section[pos].replace('__DISPLAY_NAME__',config['name'])


def assign_orientation(config,pbxproj):
    """
    Assigns the default orientation for a mobile build

    Both iPhone and iPad builds receive the same orientation by default.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``
    """
    orientation = determine_orientation(config['orientation'])
    portrait = 'Portrait' in orientation
    # XCBuildConfiguration
    section = pbxproj['XCBuildConfiguration']
    for pos in range(len(section)):
        data = section[pos].split('\n')
        change = False
        for ii in range(len(data)):
            if portrait and 'INFOPLIST_KEY_UILaunchStoryboardName' in data[ii]:
                data[ii] = '\t\t\t\tINFOPLIST_KEY_UILaunchStoryboardName = Portrait;'
                change = True
            if portrait and 'INFOPLIST_KEY_UIMainStoryboardFile' in data[ii]:
                data[ii] = '\t\t\t\tINFOPLIST_KEY_UIMainStoryboardFile = Portrait;'
                change = True
            if 'INFOPLIST_KEY_UISupportedInterfaceOrientations' in data[ii]:
                data[ii] = '\t\t\t\tINFOPLIST_KEY_UISupportedInterfaceOrientations = '+orientation+';'
                change = True
            if 'INFOPLIST_KEY_UISupportedInterfaceOrientations_iPad' in data[ii]:
                data[ii] = '\t\t\t\tINFOPLIST_KEY_UISupportedInterfaceOrientations_iPad = '+orientation+';'
                change = True
            if 'INFOPLIST_KEY_UISupportedInterfaceOrientations_iPhone' in data[ii]:
                data[ii] = '\t\t\t\tINFOPLIST_KEY_UISupportedInterfaceOrientations_iPhone = '+orientation+';'
                change = True
        if change:
            section[pos] = '\n'.join(data)


def populate_assets(config,pbxproj):
    """
    Adds the assets to the XCode project

    While assets are all collected in a single folder, we do not add that folder by
    itself. Instead we add all of the top-level entries of that folder. Files are
    added directly. Subfolders are added by reference.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``
    """
    uuidsvc = config['uuids']

    # Locate the resource entries
    mac_rsc = None
    ios_rsc   = None
    # Find the resources
    for obj in pbxproj['PBXNativeTarget']:
        pos1 = obj.find('/* Resources */')
        pos0 = obj.rfind('\n',0,pos1)
        if pos0 != -1 and pos1 != -1:
            if MAC_TARGET in obj:
                mac_rsc = obj[pos0:pos1].strip()
            elif IOS_TARGET in obj:
                ios_rsc = obj[pos0:pos1].strip()

    # Put in assets
    children = []
    macref = []
    iosref = []
    for asset in config['asset_list']:
        uuid = uuidsvc.getAppleUUID('ASSET://'+asset[0])
        uuid = uuidsvc.applyPrefix('AA',uuid)

        apath = util.path_to_posix(asset[0])
        children.append('%s /* %s */,' % (uuid,asset[0]))
        if asset[1] == 'file':
            entry = '\t\t%s /* %s */ = {isa = PBXFileReference; path = %s; sourceTree = "<group>"; };\n' % (uuid,apath,apath)
        else:
            entry = '\t\t%s /* %s */ = {isa = PBXFileReference; lastKnownFileType = folder; path = %s; sourceTree = "<group>"; };\n' % (uuid,apath,apath)
        pbxproj['PBXFileReference'].append(entry)

        newid = uuidsvc.getAppleUUID('MACOS://'+uuid)
        newid = uuidsvc.applyPrefix('AB',newid)
        entry = '\t\t%s /* %s in Resources */ = {isa = PBXBuildFile; fileRef = %s /* %s */; };\n' % (newid, apath, uuid, apath)
        macref.append('%s /* %s in Resources */,' % (newid,asset[0]))
        pbxproj['PBXBuildFile'].append(entry)

        newid = uuidsvc.getAppleUUID('IOS://'+uuid)
        newid = uuidsvc.applyPrefix('AC',newid)
        entry = '\t\t%s /* %s in Resources */ = {isa = PBXBuildFile; fileRef = %s /* %s */; };\n' % (newid, apath, uuid, apath)
        iosref.append('%s /* %s in Resources */,' % (newid,asset[0]))
        pbxproj['PBXBuildFile'].append(entry)

    # Add them to the group
    groups = pbxproj['PBXGroup']
    for pos in range(len(groups)):
        if '/* Assets */ =' in groups[pos]:
            groups[pos] = place_entries(groups[pos],children,'children')

    # Add them to the resources
    groups = pbxproj['PBXResourcesBuildPhase']
    for pos in range(len(groups)):
        if mac_rsc and mac_rsc in groups[pos]:
            groups[pos] = place_entries(groups[pos],macref)
        elif ios_rsc and ios_rsc in groups[pos]:
            groups[pos] = place_entries(groups[pos],iosref)


def populate_sources(config,pbxproj):
    """
    Adds the source code files to the XCode project

    This method builds a filetree so that are can organize subdirectories as explicit
    groups in XCode.  Note that the build script has the ability to separate source
    code into three categories: macOS only, iOS only, and both apple builds.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``
    """
    uuidsvc = config['uuids']
    filetree = config['source_tree']
    sourcedir = '../'+util.path_to_posix(config['build_to_root'])
    if len(config['source_tree']) == 1:
        key = list(config['source_tree'].keys())[0]
        sourcedir += '/'+util.path_to_posix(key)
        filetree = filetree[key]

    uuid, result = build_groups(sourcedir,filetree,uuidsvc)

    # Get the top level elements
    files = []
    tops  = result[uuid]
    entries = []
    for item in tops[1]:
        if item[2] != 'group':
            files.append(item)
        entries.append('%s /* %s */,' % (item[0],util.path_to_posix(item[1])))

    # Find the root
    groups = pbxproj['PBXGroup']
    for pos in range(len(groups)):
        if '/* Source */ =' in groups[pos]:
            groups[pos] = place_entries(groups[pos],entries,'children')

    # Append all groups that are not root
    for key in result:
        if key != uuid:
            text  = '\t\t%s /* %s */ = {\n' % (key,result[key][0])
            text += '\t\t\tisa = PBXGroup;\n\t\t\tchildren = (\n'
            for item in result[key][1]:
                if item[2] != 'group':
                    files.append(item)
                text += '\t\t\t\t%s /* %s */,\n' % (item[0],util.path_to_posix(item[1]))
            text += '\t\t\t);\n\t\t\tpath = %s;\n' % util.path_to_posix(result[key][0])
            text += '\t\t\tsourceTree = "<group>";\n\t\t};\n'
            groups.append(text)

    # Now populate the files
    macref = []
    iosref = []
    for item in files:
        ipath = util.path_to_posix(item[1])
        entry = '\t\t%s /* %s */ = {isa = PBXFileReference; fileEncoding = 4; path = %s; sourceTree = "<group>"; };\n' % (item[0],ipath,ipath)
        pbxproj['PBXFileReference'].append(entry)

        if os.path.splitext(item[1])[1] in SOURCE_EXT:
            if item[2] in ['all','apple','macos']:
                newid = uuidsvc.getAppleUUID('MACOS://'+item[0])
                newid = uuidsvc.applyPrefix('BB',newid)
                entry = '\t\t%s /* %s in Sources */ = {isa = PBXBuildFile; fileRef = %s /* %s */; };\n' % (newid, ipath, item[0], ipath)
                macref.append('%s /* %s in Sources */,' % (newid,ipath))
                pbxproj['PBXBuildFile'].append(entry)

            if item[2] in ['all','apple','ios']:
                newid = uuidsvc.getAppleUUID('IOS://'+item[0])
                newid = uuidsvc.applyPrefix('BC',newid)
                entry = '\t\t%s /* %s in Sources */ = {isa = PBXBuildFile; fileRef = %s /* %s */; };\n' % (newid, ipath, item[0], ipath)
                iosref.append('%s /* %s in Sources */,' % (newid,ipath))
                pbxproj['PBXBuildFile'].append(entry)

    # Locate the source entries
    mac_src = None
    ios_src   = None
    for obj in pbxproj['PBXNativeTarget']:
        pos1 = obj.find('/* Sources */')
        pos0 = obj.rfind('\n',0,pos1)
        if pos0 != -1 and pos1 != -1:
            if MAC_TARGET in obj:
                mac_src = obj[pos0:pos1].strip()
            elif IOS_TARGET in obj:
                ios_src = obj[pos0:pos1].strip()

    # Add them to the builds
    groups = pbxproj['PBXSourcesBuildPhase']
    for pos in range(len(groups)):
        if mac_src and mac_src in groups[pos]:
            groups[pos] = place_entries(groups[pos],macref)
        elif ios_src and ios_src in groups[pos]:
            groups[pos] = place_entries(groups[pos],iosref)


def filter_targets(config,pbxproj):
    """
    Removes any extraneous targets from XCode project if necessary.

    The build script has the option to specify a macOS only build, or an iOS
    only build. If that is the case, we remove the build that is not relevant.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``
    """
    targets = config['targets'] if type(config['targets']) == list else [config['targets']]
    if 'apple' in targets or ('ios' in targets and 'macos' in targets):
        return

    rems = []
    section = pbxproj['PBXNativeTarget']
    for pos in range(len(section)):
        if 'ios' in targets and MAC_TARGET in section[pos]:
            rems.append(pos)
        if 'macos' in targets and IOS_TARGET in section[pos]:
            rems.append(pos)

    pos = 0
    off = 0
    while pos < len(section):
        if pos+off in rems:
            del section[pos]
            off += 1
        else:
            pos += 1


def filter_modules(config,pbxproj):
    """
    Removes any extraneous library builds from XCode project if necessary.

    The latest version of CUGL breaks it up in the modules to speed up the
    build process. This removes any unnecessary modules from the link phase.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``
    """
    section = pbxproj['PBXFrameworksBuildPhase']

    # There should be two: one for macos, one for ios
    for pos in range(len(section)):
        text = section[pos]
        if text.find(MAC_TARGET):
            # Only macOS can drop graphics
            modules = {'audio':'libcugl-audio','graphics':'libcugl-graphics',
                       'scene2':'libcugl-scene2','scene3':'libcugl-scene3',
                       'physics2':'libcugl-physics2','netcode':'libcugl-netcode',
                       'physics2::distrib':'libcugl-distrib-physics2'}
        else:
            modules = {'audio':'libcugl-audio',
                       'scene2':'libcugl-scene2','scene3':'libcugl-scene3',
                       'physics2':'libcugl-physics2','netcode':'libcugl-netcode',
                       'physics2::distrib':'libcugl-distrib-physics2'}

        pos0 = text.find('files')
        pos0 = text.find('\n',pos0)+1
        pos1 = text.find('\n\t\t\t)',pos0)

        # split into lines
        alllibs = text[pos0:pos1].split('\n')
        thelibs = []
        for item in alllibs:
            skip = False
            for key in modules:
                if not config['modules'][key] and modules[key] in item:
                    skip = True
            if not skip:
                thelibs.append(item)

        thelibs = '\n'.join(thelibs)
        section[pos] = text[:pos0]+thelibs+text[pos1:]


def expand_includes(config,pbxproj):
    """
    Updates the XCode project to include the headers referenced by config

    Note that headers are broken up into three categories: 'macos', 'ios', and
    'all' (referencing both). We do not further split up headers by build
    type (e.g. 'debug' and 'release')

    :param config: The project configuration settings
    :type config:  ``dict``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``
    """
    # Expand includes
    groupdir = lambda x: '"$(SRCROOT)/../'+util.path_to_posix(config['build_to_root'])+'/'+util.path_to_posix(x)+'"'

    indent = '\t\t\t\t\t'
    entries = config['include_dict']
    if 'macos' in entries and entries['macos']:
        macset = list(set(entries['macos']))
        macset.sort()
        macincludes = indent+(',\n'+indent).join(map(groupdir,macset))+',\n'
    else:
        macincludes = ''
    if 'ios' in entries and entries['ios']:
        ioset = list(set(entries['ios']))
        ioset.sort()
        iosincludes = indent+(',\n'+indent).join(map(groupdir,ioset))+',\n'
    else:
        iosincludes = ''
    if 'all' in entries and entries['all']:
        allincludes = config['include_dict']['all']
    else:
        allincludes = []
    if 'apple' in entries and entries['apple']:
        allincludes += config['include_dict']['all']
    else:
        allincludes += []
    if allincludes:
        allset = list(set(allincludes))
        allset.sort()
        allincludes = indent+(',\n'+indent).join(map(groupdir,allset))+',\n'
    else:
        allincludes = ''

    section = pbxproj['XCBuildConfiguration']
    for pos in range(len(section)):
        if '__APPLE_INCLUDE__,' in section[pos]:
            section[pos] = section[pos].replace(indent+'__APPLE_INCLUDE__,\n',allincludes)
        if '__MACOS_INCLUDE__,' in section[pos]:
            section[pos] = section[pos].replace(indent+'__MACOS_INCLUDE__,\n',macincludes)
        if '__IOS_INCLUDE__,' in section[pos]:
            section[pos] = section[pos].replace(indent+'__IOS_INCLUDE__,\n',iosincludes)


def expand_defines(config,pbxproj):
    """
    Updates the XCode project to include the preprocessor defines referenced by config

    Note that defines are broken up into three categories: 'macos', 'ios', and
    'all' (referencing both). They are further broken up into the build types
    (e.g. 'debug' and 'release'). So there are up to six possible combinations
    here.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``
    """
    quoteit = lambda x: '"'+x+'"'
    indent = '\t\t\t\t\t'
    section = pbxproj['XCBuildConfiguration']
    entries = config['define_dict']

    targets = {'apple': ALL_TARGET,'macos':MAC_TARGET,'ios':IOS_TARGET }
    for platform in targets:
        buildconfigs = get_buildconfigs(targets[platform],pbxproj)
        for build in buildconfigs:
            for pos in range(len(section)):
                xcfg = section[pos]
                if buildconfigs[build] in xcfg:
                    if platform in entries and build in entries[platform]:
                        defset =  entries[platform][build]
                        if (len(defset) > 0):
                            defines = indent+(',\n'+indent).join(map(quoteit,defset))+',\n'
                        else:
                            defset = ''
                    else:
                        defines = ''
                    section[pos] = xcfg.replace(indent+'__EXTRA_DEFINES__,\n',defines)


def write_pbxproj(pbxproj,project):
    """
    Recollates the XCode objects back into a PBX file in the build directory.

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``

    :param project: The Apple build directory
    :type project:  ``str``
    """
    path = os.path.join(project,'project.pbxproj')
    with open(path,'w') as file:
        for state in pbxproj['PBXOrder']:
            if not state in ['PBXHeader', 'PBXFooter']:
                file.write('/* Begin '+state+' section */\n')
            for item in pbxproj[state]:
                file.write(item)
            if not state in ['PBXHeader', 'PBXFooter']:
                file.write('/* End '+state+' section */\n\n')


def update_schemes(config,project):
    """
    Updates the schemes to match the new application name.

    Note that schemes are separate files and NOT in the PBX file.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param project: The Apple build directory
    :type project:  ``str``
    """
    targets = config['targets'] if type(config['targets']) == list else [config['targets']]

    management = os.path.join(project,'xcshareddata','xcschemes','xcschememanagement.plist')
    context = { 'app-mac':config['short'].lower()+'-mac', 'app-ios':config['short'].lower()+'-ios' }
    context['__DISPLAY_NAME__'] = config['name']
    util.file_replace(management,context)

    name = config['short'].lower()+'-mac'
    src = os.path.join(project,'xcshareddata','xcschemes','app-mac.xcscheme')
    dst = os.path.join(project,'xcshareddata','xcschemes',name+'.xcscheme')
    if 'apple' in targets or 'macos' in targets:
        shutil.move(src,dst)
        context = {'app-mac':name,'container:app.xcodeproj':'container:'+config['camel']+'.xcodeproj'}
        util.file_replace(dst,context)
    else:
        os.remove(src)

    name = config['short'].lower()+'-ios'
    src = os.path.join(project,'xcshareddata','xcschemes','app-ios.xcscheme')
    dst = os.path.join(project,'xcshareddata','xcschemes',name+'.xcscheme')
    if 'apple' in targets or 'ios' in targets:
        shutil.move(src,dst)
        context = {'app-ios':name,'container:app.xcodeproj':'container:'+config['camel']+'.xcodeproj'}
        util.file_replace(dst,context)
    else:
        os.remove(src)


# THIS NEXT PART IS THE HEADACHE OF EXTERNAL LIBRARIES
# We only support this on machines with XCode

def get_buildconfigs(uuid,pbxproj):
    """
    Returns the build configuration UUIDs for the given target

    The value returned is a dictionary mapping build names (e.g. 'debug',
    'release', etc.) to the UUID for that particular build. This is necessary
    as each target has multiple build types.

    :param uuid: The target UUID
    :type uuid:  ``str``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``

    :return: The build configuration UUIDs for the given target
    :rtype:  ``dict``
    """
    # Need to find the build configuration list
    listid = None
    section = pbxproj['PBXNativeTarget']
    for item in section:
        if uuid in item:
            pos0 = item.find('buildConfigurationList')
            pos0 = item.find('=',pos0)+1
            pos1 = item.find('/*',pos0)
            listid = item[pos0:pos1].strip()

    if listid is None:
        # See if we are actually the top level configuration
        # EB0F3C8E27FB9DCB0037CC66
        section = pbxproj['PBXProject']
        for item in section:
            if uuid in item:
                pos0 = item.find('buildConfigurationList')
                pos0 = item.find('=',pos0)+1
                pos1 = item.find('/*',pos0)
                listid = item[pos0:pos1].strip()

    if listid is None:
        return None

    # Need to find the build configurations
    section = pbxproj['XCConfigurationList']
    builds = None
    for item in section:
        if listid in item:
            pos0 = item.find('buildConfigurations')
            pos0 = item.find('(',pos0)+1
            pos1 = item.find(')',pos0)
            builds = item[pos0:pos1].strip()
            builds = builds.split(',')

    if builds is None:
        return None

    # Extract the build ids
    result = {}
    for item in builds:
        if '/*' in item:
            pos0 = item.find('/*')
            pos1 = item.find('*/',pos0)
            name = item[pos0+2:pos1].strip().lower()
            guid = item[:pos0].strip()
            result[name] = guid

    return result


def extract_includes(uuid,pbxproj,path):
    """
    Returns the includes associated with the given target

    The value returned is a list of directories, all relative to the Xcode
    project. There is only one list. Unlike defines, we do not separate
    includes by build type (e.g. 'debug', 'release').

    :param uuid: The target UUID
    :type uuid:  ``str``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``

    :param path: The path to the XCode project file
    :type path:  ``str``

    :return: the includes associated with the given target
    :rtype:  ``list`` of ``str``
    """
    configs = get_buildconfigs(uuid,pbxproj)

    # We are NOT going to bother to separate the includes. Merge them
    includes = []
    section = pbxproj['XCBuildConfiguration']
    for item in section:
        for build in configs:
            if configs[build] in item and 'HEADER_SEARCH_PATHS' in item:
                pos0 = item.find('HEADER_SEARCH_PATHS')
                pos0 = item.find('(',pos0)+1
                pos1 = item.find(');',pos0)
                includes.extend(item[pos0:pos1].split(','))

    # Remove duplicates
    includes = list(set(includes))

    # Normalize the includes
    result = []
    for item in includes:
        if item[0] in '\'"':
            item = item[1:-1]

        if not '$(inherited)' in item:
            # Construct an absolute path
            if '$(SRCROOT)/' in item:
                item = item.replace('$(SRCROOT)/',path)
            if len(item) == 0 or item[0] != '/':
                item = path+'/'+item

            relp = os.path.relpath(item,path)
            result.append(relp)

    return result


def extract_defines(uuid,pbxproj):
    """
    Returns the defines associated with the given target

    The value returned is a dictionary mapping build types (e.g. 'debug',
    'release') to a list of preprocessor defines (strings).

    :param uuid: The target UUID
    :type uuid:  ``str``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``

    :return: the defines associated with the given target
    :rtype:  ``dict``
    """
    configs = get_buildconfigs(uuid,pbxproj)

    # Defines are grouped by build type
    result = {}
    section = pbxproj['XCBuildConfiguration']
    for item in section:
        for build in configs:
            if configs[build] in item and 'GCC_PREPROCESSOR_DEFINITIONS' in item:
                pos0 = item.find('GCC_PREPROCESSOR_DEFINITIONS')
                pos0 = item.find('(',pos0)+1
                pos1 = item.find(')',pos0)

                defines = item[pos0:pos1].split(',');

                # Remove unnecessary defines
                keeping = []
                for defval in defines:
                    if item == '$(inherited)':
                        pass
                    elif 'CMAKE_INTDIR' in item:
                        pass
                    else:
                        keeping.append(defval)

                result[build] = keeping

    return result


def extract_frameworks(path):
    """
    Returns a dictionary of frameworks mentioned in the given XCode directory.

    A CMake project may need to link against macOS/iOS frameworks. However, as
    the generated XCode project is not a final application (that would be our
    XCode project), it does not have that information. Fortunately, that
    information is found in the 'CMakeCache.txt' file. This function searches
    that file for any necessary libraries.

    The value returned in a dictionary mapping framework name to its path
    in the SDK/developer directory.

    :param path: The path to the generated XCode directory
    :type path:  ``str``

    :return: a dictionary of frameworks mentioned in the given XCode directory.
    :rtype:  ``dict``
    """
    cache = os.path.join('CMakeCache.txt')
    if not os.path.isfile(cache):
        return None

    libs = []
    with open(cache) as file:
        next = False
        for line in file:
            if next:
                if not 'NOTFOUND' in line:
                    libs.append(line.strip())
                next = False
            elif 'Path to a library' in line:
                next = True

    results = {}
    for item in libs:
        pos0 = item.find('FILEPATH')
        if pos0 != -1:
            pos0 = item.find('=',pos0)+1
            item = item[pos0:]
            if 'Developer/Platforms' in item:
                pos0 = item.find('Developer/Platforms')
                pos0 = item.find('/',pos0)+1
                item = item[pos0:]

            name = item
            if '/' in name:
                pos0 = name.rfind('/')
                name = name[pos0+1:]

            results[name] = item

    return results


def expand_targets(target,targetset,pbxproj):
    """
    Recursively expands the targets associated with the given root target

    When a student links in an external library, they typically only specify
    the top level target. However, that target often has other dependencies
    that can be found in the XCode project. Those targets need to be added to
    the target to ensure that the final application links against all of these
    dependencies. This function is recursive, ensuring that all dependencies
    are met.

    All data is stored in the targetset. This dictionary maps target UUIDs
    to a pair of name (as referenced in XCode) and path (the library built by
    the target).

    :param target: The target name OR UUID
    :type target:  ``str``

    :param targetset: The dictionary to store the results
    :type targetset:  ``dict``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``
    """
    uuid = None
    for pos in range(len(pbxproj['PBXFileReference'])):
        line = pbxproj['PBXFileReference'][pos]
        if 'BUILT_PRODUCTS_DIR' in line and target in line:
            pos0 = line.find('/')
            uuid = line[:pos0].strip()
            pos0 = line.find('*',pos0)+1
            pos1 = line.find('*/',pos0)
            name = line[pos0:pos1].strip()
            if name and name[0] in '\'"':
                name = name[1:-1]
            pos0 = line.find('path',pos1)
            pos0 = line.find('=',pos0)+1
            pos1 = line.find(';',pos0)
            path = line[pos0:pos1].strip()
            if path and path[0] in '\'"':
                path = path[1:-1]
            targetset[uuid] = (name,path)
            break

    if uuid is None:
        return

    # Find any candidate dependencies
    section = pbxproj['PBXNativeTarget']
    candidates = []
    for item in section:
        if 'productReference = '+uuid in item and 'dependencies = (' in item:
            pos0 = item.find('dependencies = (')
            pos0 = item.find('(',pos0)+1
            pos1 = item.find(')',pos0)
            deplist = item[pos0:pos1].strip()
            if deplist:
                deplist = deplist.split('\n')
                for depn in deplist:
                    if '/' in depn:
                        pos0 = depn.find('/')
                        depn = depn[:pos0]
                    depn = depn.strip()
                    if depn:
                        candidates.append(depn)

    identified = []
    section = pbxproj['PBXTargetDependency']
    for item in section:
        for depn in candidates:
            if depn in item:
                pos0 = item.find('target =')
                pos0 = item.find('=',pos0)+1
                pos1 = item.find('/',pos0)
                found = item[pos0:pos1].strip()
                identified.append(found)

    section = pbxproj['PBXNativeTarget']
    for item in section:
        for found in identified:
            if found in item:
                pos0 = item.find('productReference =')
                pos0 = item.find('=',pos0)+1
                pos1 = item.find('/',pos0)
                duuid = item[pos0:pos1].strip()
                if duuid and not duuid in targetset:
                    expand_targets(duuid,targetset,pbxproj)


def configure_external(config,module,xcode,target):
    """
    Returns a configuration dictionary for linking in an external Xcode project

    This function does two things. First, it reconfigures the given Xcode project
    to identify it as either macOS or iOS (CMake will generate identically
    looking projects, so we need to be able to distinguish them).

    After the reconfiguration, it extracts the relevant information for each
    target.  It gets the UUID identifying the target in the give Xcode project.
    And then it generates some new UUIDs for linking into the master project.

    The returned data structure is a dictionary as there is A LOT of information
    necessary for this linking.  The key '' maps to top-level information about
    the Xcode project itself, while each target name is a key to the associated
    linking information.  Note that dynamically linked libraries like Frameworks
    are interpreted as top-level information since that information is extracted
    from the cache where target association is less clear.

    :param config: The project configuration settings
    :type config:  ``dict``

    :param module: The external module name
    :type xcode:  ``str``

    :param xcode: The path to the Xcode project for this module
    :type xcode:  ``str``

    :param target: The target to link this module to
    :type target:  ``str`` (on of MAC_TARGET, IOS_TARGET)

    :return: the dictionary with the linking information
    :rtype:  ``dict``
    """
    if not 'targets' in module:
        return

    uuidsvc = config['uuids']
    platform = 'macos' if target == MAC_TARGET else 'ios'
    suffix = '-mac' if target == MAC_TARGET else '-ios'

    pos = xcode.rfind('.')
    if pos == -1:
        return

    # Copy the project to separate macOS from iOS
    project = xcode[:pos]+suffix+xcode[pos:]
    shutil.move(xcode, project)

    pbxproj = parse_pbxproj(project)

    # The root level of the result is the project information
    path, file = os.path.split(project)
    root = {}
    root['name'] = module['name']
    root['project'] = project
    root['file'] = file
    root['path'] = path
    root['uuids'] = {}
    uuid = uuidsvc.getAppleUUID('PROJECT://'+project)
    root['uuids']['project'] = uuidsvc.applyPrefix('CA',uuid)
    uuid = uuidsvc.getAppleUUID('CONTAINER://'+project)
    root['uuids']['container'] = uuidsvc.applyPrefix('CB',uuid)
    result = {'':root}

    # Get the FULL list of targets
    targetset = {}
    for target in module['targets']:
        expand_targets(target,targetset,pbxproj)

    # Now get information for each target
    for uuid in targetset:
        name = targetset[uuid][0]
        path = targetset[uuid][1]
        if path[-2:] != '.a':
            print('ERROR: Could not find static library for %s in %s' % (name,project))
            break

        # Find any link information available (to store in config)
        includes = extract_includes(uuid,pbxproj,config['root'])
        if includes:
            if platform in config['include_dict']:
                config['include_dict'][platform].extend(includes)
            else:
                config['include_dict'][platform] = includes

        # Find any define information available (to store in config)
        defines = extract_defines(uuid,pbxproj)
        if defines:
            if platform in config['define_dict']:
                config['define_dict'][platform] = util.merge_dict(config['define_dict'][platform],defines)
            else:
                config['define_dict'][platform] = defines

        # Rename the module everywhere
        newname = name+suffix
        newpath = path[:-2]+suffix+path[-2:]
        for pos in range(len(pbxproj['PBXFileReference'])):
            line = pbxproj['PBXFileReference'][pos]
            if uuid in line:
                pos0 = line.find('path = ')
                line0 = line[:pos0].replace(name,newname)
                line1 = line[pos0:].replace(path,newpath)
                pbxproj['PBXFileReference'][pos] = line0+line1

        for pos in range(len(pbxproj['PBXGroup'])):
            line = pbxproj['PBXGroup'][pos]
            if uuid in line:
                pbxproj['PBXGroup'][pos] = line.replace(name,newname)

        for pos in range(len(pbxproj['PBXNativeTarget'])):
            line = pbxproj['PBXNativeTarget'][pos]
            if uuid in line:
                pbxproj['PBXNativeTarget'][pos] = line.replace(name,newname)

        # Gather the module information
        result[name] = {}
        result[name]['name'] = newname
        result[name]['path'] = newpath
        result[name]['uuids'] = {'external': uuid}

        # We need A LOT of UUIDs. Generate them now.
        uuid = uuidsvc.getAppleUUID('LIBRARY://'+newname)
        result[name]['uuids']['file'] = uuidsvc.applyPrefix('CC',uuid)
        uuid = uuidsvc.getAppleUUID('PROXY://'+newname)
        result[name]['uuids']['proxy'] = uuidsvc.applyPrefix('CD',uuid)
        uuid = uuidsvc.getAppleUUID('CONTAINER://'+newname)
        result[name]['uuids']['container'] = uuidsvc.applyPrefix('CE',uuid)

    # Extract any frameworks we need
    frameworks = extract_frameworks(path)
    if frameworks:
        # Time to make a lot more UUIDs
        root['frameworks']= {}
        for name in frameworks:
            root['frameworks'][name] = {}
            root['frameworks'][name]['path'] = frameworks[name]
            uuid = uuidsvc.getAppleUUID('FRAMEWORK://'+frameworks[name])
            root['frameworks'][name]['fileuuid'] = uuidsvc.applyPrefix('DA',uuid)
            uuid = uuidsvc.getAppleUUID('REFERENCE://'+frameworks[name])
            root['frameworks'][name]['refuuid'] = uuidsvc.applyPrefix('DB',uuid)

    write_pbxproj(pbxproj,project)
    return result


def link_external(pbxproj,libraries,target):
    """
    Links the given libraries into the given pbxproj

    Libraries should be a data structure generated by configure_external_libraries
    which has all of the information necessary for linking.

    This code is an absolute mess. The only reason I am doing this is because
    students begged for this feature for years.

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``

    :param libraries: The library link information
    :type libraries: ``dict``

    :param target: The target to link to
    :type target:  ``str`` (on of MAC_TARGET, IOS_TARGET)
    """
    # Extract root information to link in XCode project
    name = libraries['']['name']
    file = libraries['']['file']
    projuuid = libraries['']['uuids']['project']
    contuuid = libraries['']['uuids']['container']

    platform = 'macos' if target == MAC_TARGET else 'ios'
    projfmt  = '\t\t%s /* %s */ = {isa = PBXFileReference; '
    projfmt += 'lastKnownFileType = "wrapper.pb-project"; name = "%s"; '
    projfmt += 'path = "%s/%s/%s"; sourceTree = "<group>"; };\n'
    pbxproj['PBXFileReference'].append(projfmt % (projuuid,file,file,platform,name,file))

    # Add the project to the main group
    for pos in range(len(pbxproj['PBXGroup'])):
        line = pbxproj['PBXGroup'][pos]
        if 'cugl.xcodeproj' in line:
            pos0 =  line.find('\t\t\t);')
            if pos0 != -1:
                text = '\t\t\t\t'+projuuid+' /* '+file+' */,\n'
                pbxproj['PBXGroup'][pos] = line[:pos0]+text+line[pos0:]

    # Add the project to a product group
    for pos in range(len(pbxproj['PBXProject'])):
        line = pbxproj['PBXProject'][pos]
        if 'cugl.xcodeproj' in line:
            # Assume template well-formed (no error checking)
            insert = line.find('projectReferences')
            insert = line.find('},\n',insert)+3
            reference = '\t\t\t\t{\n'
            reference += '\t\t\t\t\tProductGroup = '+contuuid+' /* Products */;\n'
            reference += '\t\t\t\t\tProjectRef = '+projuuid+' /* '+file+' */;\n'
            reference += '\t\t\t\t},\n'
            pbxproj['PBXProject'][pos] = line[:insert]+reference+line[insert:]

    # Container formatting string
    containerfmt  = '\t\t%s /* PBXContainerItemProxy */ = {\n'
    containerfmt += '\t\t\tisa = PBXContainerItemProxy;\n'
    containerfmt += '\t\t\tcontainerPortal = %s /* %s */;\n'
    containerfmt += '\t\t\tproxyType = 2;\n'
    containerfmt += '\t\t\tremoteGlobalIDString = %s;\n'
    containerfmt += '\t\t\tremoteInfo = "%s";\n'
    containerfmt += '\t\t};\n'

    # Proxy formatting string
    proxyfmt  = '\t\t%s /* %s */ = {\n'
    proxyfmt += '\t\t\tisa = PBXReferenceProxy;\n'
    proxyfmt += '\t\t\tfileType = archive.ar;\n'
    proxyfmt += '\t\t\tpath = "%s";\n'
    proxyfmt += '\t\t\tremoteRef = %s /* PBXContainerItemProxy */;\n'
    proxyfmt += '\t\t\tsourceTree = BUILT_PRODUCTS_DIR;\n'
    proxyfmt += '\t\t};\n'

    # Formatting string for each target
    filefmt = '\t\t%s /* %s in Frameworks */ = {isa = PBXBuildFile; fileRef = %s /* %s */; };\n'

    # Now process the individual targets
    for lib in libraries:
        if lib != '':
            libname  = libraries[lib]['name']
            library  = libraries[lib]['path']
            groupuuid = libraries[lib]['uuids']['container']
            fileuuid  = libraries[lib]['uuids']['file']
            proxyuuid = libraries[lib]['uuids']['proxy']
            refuuid   = libraries[lib]['uuids']['external']

            pbxproj['PBXContainerItemProxy'].append(containerfmt % (groupuuid, projuuid, file, refuuid, libname))
            pbxproj['PBXReferenceProxy'].append(proxyfmt % (proxyuuid, library, library, groupuuid))
            pbxproj['PBXBuildFile'].append(filefmt % (fileuuid,library,proxyuuid,library))


    # Group all targets together
    pbxgroup = '\t\t'+contuuid+' /* Products */ = {\n'
    pbxgroup += '\t\t\tisa = PBXGroup;\n'
    pbxgroup += '\t\t\tchildren = (\n'

    pbxlink = ''
    for lib in libraries:
        if lib != '':
            library  = libraries[lib]['path']
            fileuuid  = libraries[lib]['uuids']['file']
            proxyuuid = libraries[lib]['uuids']['proxy']

            pbxgroup += '\t\t\t\t'+proxyuuid+' /* '+library+' */,\n'
            pbxlink += '\t\t\t\t'+fileuuid+' /* '+library+' in Frameworks */,\n'

    pbxgroup += '\t\t\t);\n'
    pbxgroup += '\t\t\tname = Products;\n'
    pbxgroup += '\t\t\tsourceTree = "<group>";\n'
    pbxgroup += '\t\t};\n'
    pbxproj['PBXGroup'].append(pbxgroup)

    # Find the build phase the current target
    pbxframeworks = ''
    for pos in range(len(pbxproj['PBXNativeTarget'])):
        line = pbxproj['PBXNativeTarget'][pos]
        if target in line:
            pos0 = line.find('/* Frameworks */')
            line = line[:pos0]
            pos0 = line.rfind('\n')
            pbxframeworks = line[pos0:].strip()

    # Add the links to the build phase
    for pos in range(len(pbxproj['PBXFrameworksBuildPhase'])):
        line = pbxproj['PBXFrameworksBuildPhase'][pos]
        if pbxframeworks in line:
            pos0 = line.find('files')
            pos0 = line.find('\n',pos0)+1
            pbxproj['PBXFrameworksBuildPhase'][pos] = line[:pos0]+pbxlink+line[pos0:]


def link_frameworks(pbxproj,libraries,target):
    """
    Links the necessary frameworks into the given pbxproj

    Libraries should be a data structure generated by configure_external_libraries
    which has all of the information necessary for linking.

    This code is an absolute mess. As with link_external, the only reason I am
    doing this is because students begged for this feature for years.

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``

    :param libraries: The library link information
    :type libraries: ``dict``

    :param target: The target to link to
    :type target:  ``str`` (on of MAC_TARGET, IOS_TARGET)
    """
    if not 'frameworks' in libraries['']:
        return

    frameworks = libraries['']['frameworks']

    # Eliminating any libraries that already exist
    platform = 'macos' if target == MAC_TARGET else 'ios'

    section = pbxproj['PBXGroup']
    for pos in range(len(section)):
        lines = section[pos].split('\n')
        if platform in lines[0]:
            idx = pos
            break

    eliminate = []
    for name in frameworks:
        if name in section[idx]:
            eliminate.append(name)
        if not (name.endswith('.tbd') or name.endswith('.framework')):
            eliminate.append(name)

    for name in eliminate:
        del frameworks[name]

    # Add the remainder to the PBXGroup
    grpfmt = '\t\t\t\t%s /* %s */,'
    grouprefs = []
    for name in frameworks:
        grouprefs.append(grpfmt % (frameworks[name]['refuuid'],name))

    try:
        end = lines.index('\t\t\t);')
        lines = lines[:end]+grouprefs+lines[end:]
        section[idx] = '\n'.join(lines)
    except:
        pass

    # Add to PBXFileReference
    reffmt  = '\t\t%s /* %s */ = {isa = PBXFileReference; '
    reffmt += 'lastKnownFileType = %s; name = "%s"; path = %s; '
    reffmt += 'sourceTree = DEVELOPER_DIR; };\n'

    section = pbxproj['PBXFileReference']
    for name in frameworks:
        if name.endswith('.tbd'):
            filetype = '"sourcecode.text-based-dylib-definition"'
        else:
            filetype = 'wrapper.framework'
        section.append(reffmt % (frameworks[name]['refuuid'],name,filetype,name,frameworks[name]['path']))

    # Add to PBXBuildFile
    filefmt  = '\t\t%s /* %s in Frameworks */ = {isa = PBXBuildFile; fileRef = %s /* %s */; };\n'

    section = pbxproj['PBXBuildFile']
    for name in frameworks:
        section.append(filefmt % (frameworks[name]['fileuuid'],name,frameworks[name]['refuuid'],name))

    # Add reference to the correct folder
    section = pbxproj['PBXGroup']
    for pos in range(len(section)):
        lines = section[pos].split('\n')
        if platform in lines[0]:
            try:
                idx = lines.index('\t\t\t);')
                lines = lines[:idx]+grouprefs+lines[idx:]
                section[pos] = '\n'.join(lines)
            except:
                pass

    # Add file to the correct build
    section = pbxproj['PBXNativeTarget']
    frwkuuid = None
    for item in section:
        if target in item:
            pos1 = item.find('/* Frameworks */,')
            pos0 = item[:pos1].rfind(',')+1
            frwkuuid = item[pos0:pos1].strip()

    if frwkuuid is None:
        return

    frwkfmt = '\t\t\t\t%s /* %s in Frameworks */,'
    frwkrefs = []
    for name in frameworks:
        frwkrefs.append(frwkfmt % (frameworks[name]['fileuuid'],name))

    section = pbxproj['PBXFrameworksBuildPhase']
    for pos in range(len(section)):
        lines = section[pos].split('\n')
        if frwkuuid in lines[0]:
            try:
                end = lines.index('\t\t\t);')
                lines = lines[:end]+frwkrefs+lines[end:]
                section[pos] = '\n'.join(lines)
            except:
                pass


def add_macos_externals(config,pbxproj):
    """
    Adds any external modules to the macOS target

    :param config: The project configuration settings
    :type config:  ``dict``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``
    """
    build = os.path.join(config['build'],MAKEDIR)
    if not os.path.exists(build):
        os.mkdir(build)

    # Create a macos subfolder if necessary
    macosdir = os.path.join(build,'macos')
    if not os.path.exists(macosdir):
        os.mkdir(macosdir)

    prefix = ['..','..','..']

    # We need to reset the working directory when done
    uuidsvc = config['uuids']
    cwd = os.getcwd()
    for item in config['external']:
        print('-- Expanding %s' % repr(item['name']))
        itemdir = os.path.join(macosdir,item['name'])
        if not os.path.exists(itemdir):
            os.mkdir(itemdir)
        os.chdir(itemdir)
        path = os.path.join(*prefix,config['build_to_project'],item['path'])

        command = ['cmake']
        if 'options' in item:
            for opt in item['options']:
                command.append('-D'+opt+'='+str(item['options'][opt]))

        command.append('-DCMAKE_OSX_DEPLOYMENT_TARGET=12.1')
        command.append('-G Xcode')
        command.append(path)
        subprocess.run(command)

        # Locate the xcode project
        xcode = glob.glob(os.path.join(itemdir,'*.xcodeproj'))
        if (len(xcode) == 0):
            print('   ERROR: No Xcode project generated for %s' % repr(item['name']))
            xcode = None
        elif (len(xcode) > 1):
            print('   WARNING: Found multiple Xcode projects for %s' % repr(item['name']))
            xcode = xcode[0]
        else:
            xcode = xcode[0]

        # If the project exists, link it up
        if xcode:
            libraries = configure_external(config,item,xcode,MAC_TARGET)
            link_external(pbxproj,libraries,MAC_TARGET)
            link_frameworks(pbxproj,libraries,MAC_TARGET)

    # Restore the working directory
    os.chdir(cwd)


def add_ios_externals(config,pbxproj):
    """
    Adds any external modules to the iOS target

    :param config: The project configuration settings
    :type config:  ``dict``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``
    """
    build = os.path.join(config['build'],MAKEDIR)
    if not os.path.exists(build):
        os.mkdir(build)

    # Create a macos subfolder if necessary
    iosdir = os.path.join(build,'ios')
    if not os.path.exists(iosdir):
        os.mkdir(iosdir)

    prefix = ['..','..','..']

    # We need to reset the working directory when done
    cwd = os.getcwd()
    for item in config['external']:
        itemdir = os.path.join(iosdir,item['name'])
        if not os.path.exists(itemdir):
            os.mkdir(itemdir)
        os.chdir(itemdir)
        path = os.path.join(*prefix,config['build_to_project'],item['path'])
        tool = os.path.join(*prefix,config['build_to_cugl'],'tools','ios.toolchain.cmake')

        command = ['cmake']
        if 'options' in item:
            for opt in item['options']:
                command.append('-D'+opt+'='+str(item['options'][opt]))

        command.append('-DCMAKE_TOOLCHAIN_FILE=%s' % tool)
        command.append('-DPLATFORM=OS64COMBINED')
        command.append('-DDEPLOYMENT_TARGET=13.1')
        command.append('-G Xcode')
        command.append(path)
        subprocess.run(command)

        # Locate the xcode project
        xcode = glob.glob(os.path.join(itemdir,'*.xcodeproj'))
        if (len(xcode) == 0):
            print('   ERROR: No Xcode project generated for %s' % repr(item['name']))
            xcode = None
        elif (len(xcode) > 1):
            print('   WARNING: Found multiple Xcode projects for %s' % repr(item['name']))
            xcode = xcode[0]
        else:
            xcode = xcode[0]


        # If the project exists, link it up
        if xcode:
            libraries = configure_external(config,item,xcode,IOS_TARGET)
            link_external(pbxproj,libraries,IOS_TARGET)
            link_frameworks(pbxproj,libraries,IOS_TARGET)

    # Restore the working directory
    os.chdir(cwd)


def add_externals(config,pbxproj):
    """
    Adds any external modules to this Xcode project

    :param config: The project configuration settings
    :type config:  ``dict``

    :param pbxproj: The dictionary of XCode objects
    :type pbxproj:  ``dict``
    """
    if not 'external' in config:
        return

    if util.get_dev_platform() != 'macos':
        print('   [SKIP]: External libraries require macOS for XCode integration')
        return

    targets = config['targets'] if type(config['targets']) == list else [config['targets']]
    if 'apple' in targets or 'macos' in targets:
        add_macos_externals(config,pbxproj)
    if 'apple' in targets or 'ios' in targets:
        add_ios_externals(config,pbxproj)


def make(config):
    """
    Creates the XCode project

    This only creates the XCode project; it does not actually build the project. To
    build the project, you must open it up in XCode.

    :param config: The project configuration settings
    :type config:  ``dict``
    """
    print()
    print('Configuring Apple build files')
    print('-- Copying XCode project')
    make_directory(config)
    project = place_project(config)
    pbxproj = parse_pbxproj(project)
    print('-- Modifying project settings')
    reassign_pbxproj(config,pbxproj)
    assign_orientation(config,pbxproj)
    print('-- Populating project file')
    populate_assets(config,pbxproj)
    populate_sources(config,pbxproj)
    print('-- Configuring modules')
    filter_modules(config,pbxproj)
    add_externals(config,pbxproj)
    expand_includes(config,pbxproj)
    expand_defines(config,pbxproj)
    print('-- Retargeting builds')
    filter_targets(config,pbxproj)
    write_pbxproj(pbxproj,project)
    update_schemes(config,project)

    if 'icon' in config:
        print('-- Generating icons')
        res = os.path.split(project)[0]
        res = os.path.join(res,'Resources')
        config['icon'].gen_macos(res)
        config['icon'].gen_ios(res)
