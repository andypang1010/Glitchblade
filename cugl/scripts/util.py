"""
Utility functions the build script

This module is a collection of utility functions that are used by all the build 
scripts.

The primary thing in this module is a collection of path functions.  Windows
makes everything complicated.  It needs its own paths format (e.g. \\ instead 
of /). But to make matters worse, "sometimes" is uses posix format. So it is 
not just a matter of using os.path, especially since we want Windows to be able
to generate build files for Apple and vice versa.

Author:  Walker M. White
Version: 7/10/24
"""
import os, os.path
import glob
import shutil
import pathlib
import platform
import shortuuid
import uuid


# Salt value to help with UUID collisions
SALT = "__sdlapp__"

#mark UUIDS

class UUIDService(object):
    """
    This class is a UUID services that creates (guaranteed) unique ids
    
    We use this service to create identifiers for XCode and Visual Studio.
    All UUIDs are guaranteed to be unique BEFORE prefixes are applied.
    Applying a prefix may sabotague uniqueness (at which case it is the
    responsibility of the application to resolve collisions).
    
    All UUIDs use hexadecimal representation. Apple uses a 24 character
    version with no dashes. Microsoft uses the traditional RFC 4122
    representation.
    """
    # The maximum number of retries on failure.
    MAX_TRIES = 8
    
    # The size of an Apple UUID
    APPLE_SIZE = 24
    
    def __init__(self):
        """
        Initializes a new UUID service
        """
        self._uniques = set()
        self._apples  = {}
        self._windows = {}
        self._shortus = shortuuid.ShortUUID(alphabet='abcdefABCDEF0123456789')
    
    def applyPrefix(self,prefix,uuid):
        """
        Returns a copy of UUID with the given prefix applied.
        
        The prefix is not concatenated, but instead is substituted
        for the beginning of the uuid. Any non-hexadecimal characters
        (such as dashed) are ignored for purposes of substitution.
        If prefix is longer than the UUID, it will be truncated to
        fit.
        
        Applying a prefix ruins all uniqueness guarantees for the UUID.
        It is up to the application designer to check for uniqueness
        after applying a prefix.
        
        :param prefix: The prefix to apply
        :type prefix:  ``str``
        
        :param uuid: The UUID (Apple or RFC 4122) to modify
        :type uuid:  ``str``
        
        :return: a copy of UUID with the given prefix applied.
        :rtype:  ``str``
        """
        copy = ''
        for ii in range(max(len(prefix),len(uuid))):
            if ii < len(uuid) and uuid[ii] == '-':
                copy += uuid[ii]
            elif ii < len(prefix):
                copy += prefix[ii]
            else:
                copy += uuid[ii]
        
        return copy[:len(uuid)]
    
    def getAppleUUID(self,url):
        """
        Returns an XCode compliant UUID
        
        An XCode compliant UUID is a 24 character hexadecimal string.
        
        This method will raise a RuntimeError if it is impossible generate
        a UUID unique from any generated before for a different URL. While
        While this possibility is vanishingly rare, applications should
        be aware of the possibility.
        
        :param url: The url seed for the UUID
        :type url:  ``str``
        
        :return: an XCode compliant UUID
        :rtype:  ``str``
        """
        # See if we have already computed it
        if url in self._apples:
            return self._apples[url]
        
        result = self._compute_Apple_UUID_(url,0)
        
        # Make sure no reuse
        if result:
            self._uniques.add(result)
            self._apples[url] = result
        return result
    
    def _compute_Apple_UUID_(self,url,tries):
        """
        Recursive helper to getAppleUUID
        
        This uses rehashing to retry UUIDs on collision. However, it will
        give up after MAX_TRIES.
        
        :param url: The url seed for the UUID
        :type url:  ``str``
        
        :param tries: The number of tries so far
        :type tries:  ``int``
        
        :return: an XCode compliant UUID
        :rtype:  ``str``
        """
        if tries >= self.MAX_TRIES:
            raise RuntimeError('ERROR: Unable to get unique UUID for %s' % repr(url))
        
        result = self._shortus.uuid(name=url+'/'+SALT).upper()[:self.APPLE_SIZE]
        if result in self._uniques:
            # We have a problem.  Hope this is rare
            return self._compute_Apple_UUID_(url,tries+1)
        
        return result
    
    def getWindowsUUID(self,url):
        """
        Returns a Visual Studio compliant UUID
        
        An Visual Studio compliant UUID meets RFC 4122 requirements. It has
        the form XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX.
        
        This method will raise a RuntimeError if it is impossible generate
        a UUID unique from any generated before for a different URL. While
        While this possibility is vanishingly rare, applications should
        be aware of the possibility.
        
        :param url: The url seed for the UUID
        :type url:  ``str``
        
        :return: a Visual Studio compliant UUID
        :rtype:  ``str``
        """
        if url in self._windows:
            return self._windows[url]
        
        result = self._compute_Windows_UUID_(url,0)
        
        # Make sure no reuse
        if result:
            self._uniques.add(result)
            self._windows[url] = result
        return result
    
    def _compute_Windows_UUID_(self,url,tries):
        """
        Recursive helper to getWindowsUUID
        
        This uses rehashing to retry UUIDs on collision. However, it will
        give up after MAX_TRIES.
        
        :param url: The url seed for the UUID
        :type url:  ``str``
        
        :param tries: The number of tries so far
        :type tries:  ``int``
        
        :return: a Visual Studio compliant UUID
        :rtype:  ``str``
        """
        if tries >= self.MAX_TRIES:
            raise RuntimeError('ERROR: Unable to get unique UUID for %s' % repr(url))
        
        result = uuid.uuid5(uuid.NAMESPACE_URL,url+'/'+SALT)
        result = str(result).upper()
        
        if result in self._uniques:
            # We have a problem.  Hope this is rare
            return self._compute_Windows_UUID_(url,tries+1)
        
        return result


pass
#mark -
#mark PATHS

def path_to_posix(path):
    """
    Returns a POSIX normalization of path, no matter the platform.
    
    When manipulating files, we typically need to use the OS-specific path
    tools needed for Python. But the various build files need platform
    specific paths. For example, CMake and XCode prefer posix paths, while
    Visual Studio needs Windows paths. This function allows us to create
    uniform build files, not matter the platform.
    
    The path in this function is unchanged on systems with POSIX paths.
    
    :param path: The path to normalize
    :type path:  ``str``
    
    :return: a POSIX compliant version of the path
    :rtype:  ``str``
    """
    if any(platform.win32_ver()):
        return windows_to_posix(path)
    return path


def path_to_windows(path,mount='C'):
    """
    Returns a Windows-specific normalization of path, no matter the platform.
    
    When manipulating files, we typically need to use the OS-specific path
    tools needed for Python. But the various build files need platform
    specific paths. For example, CMake and XCode prefer posix paths, while
    Visual Studio needs Windows paths. This function allows us to create
    uniform build files, not matter the platform.
    
    The path in this function is unchanged on Windows systems. On POSIX
    systems, absolute paths are mapped to the given (optional) mount point.
    
    :param path: The path to normalize
    :type path:  ``str``
    
    :param mount: The mount point for absolute paths
    :type mount:  ``str``
    
    :return: a Wdinows compliant version of the path
    :rtype:  ``str``
    """
    if any(platform.win32_ver()):
        return path
    return posix_to_windows(path,mount)


def posix_to_path(posix,mount='C'):
    """
    Returns a platform-specific path from a POSIX path
    
    When manipulating files, we typically need to use the OS-specific path
    tools needed for Python. But the various build files need platform
    specific paths. For example, CMake and XCode prefer posix paths, while
    Visual Studio needs Windows paths. This function allows us to create
    uniform build files, not matter the platform.
    
    On a Windows machine, any absolute path will map to the given (optional)
    mount point
    
    :param path: The path to platformize
    :type path:  ``str``
    
    :param mount: The mount point for absolute paths
    :type mount:  ``str``
    
    :return: a platform-specific version of the POSIX path
    :rtype:  ``str``
    """
    if any(platform.win32_ver()):
        return posix_to_windows(posix,mount)
    return posix


def windows_to_path(path):
    """
    Returns a platform-specific path from a Windows path
    
    When manipulating files, we typically need to use the OS-specific path
    tools needed for Python. But the various build files need platform
    specific paths. For example, CMake and XCode prefer posix paths, while
    Visual Studio needs Windows paths. This function allows us to create
    uniform build files, not matter the platform.
    
    The path in this function is unchanged on Windows systems. On POSIX
    systems, absolute paths ignore the mount point and are mapped to the root.
    
    :param path: The path to platformize
    :type path:  ``str``
    
    :return: a platform-specific version of the Windows path
    :rtype:  ``str``
    """
    if any(platform.win32_ver()):
        return path
    return windows_to_posix(path)


def windows_to_posix(path):
    """
    Returns a POSIX normalization of the given Windows path.
    
    When manipulating files, we typically need to use the OS-specific path
    tools needed for Python. But the various build files need platform
    specific paths. For example, CMake and XCode prefer posix paths, while
    Visual Studio needs Windows paths. This function allows us to create
    uniform build files, not matter the platform.
    
    Absolute paths will disregard the mount point and map to the root.
    
    :param path: The path to normalize
    :type path:  ``str``
    
    :return: a POSIX compliant version of the path
    :rtype:  ``str``
    """
    pos = path.find(':')
    if pos != -1:
        path = path[pos+1:]
    return pathlib.PureWindowsPath(path).as_posix()


def posix_to_windows(posix,mount='C'):
    """
    Returns a Windows path from a POSIX path
    
    When manipulating files, we typically need to use the OS-specific path
    tools needed for Python. But the various build files need platform
    specific paths. For example, CMake and XCode prefer posix paths, while
    Visual Studio needs Windows paths. This function allows us to create
    uniform build files, not matter the platform.
    
    Any absolute path will map to the given (optional) mount point
    
    :param path: The path to platformize
    :type path:  ``str``
    
    :param mount: The mount point for absolute paths
    :type mount:  ``str``
    
    :return: a platform-specific version of the POSIX path
    :rtype:  ``str``
    """
    components = posix.split('/')
    if len(components) == 0:
        return posix
    else:
        if components[0] == '':
            return mount+':'+'\\'.join(components)
        else:
            return '\\'.join(components)
    
    return posix


def posix_split(path):
    """
    Returns a list with the components of a POSIX path description
    
    This function is necessary because the config files will express paths 
    using POSIX separators. All absolute paths will start with an empty
    element at the front of the list.  This is different from an empty path,
    which is represented by an empty list.
    
    :param path: The path to split
    :type path:  ``str``
    
    :return: the list of the path components
    :rtype:  ``list``
    """
    result = path.split('/')
    return result


def windows_split(path):
    """
    Returns a list with the components of a Windows path description

    THis function is necessary because sometimes we need to be working with 
    Windows paths on non-Windows machines. All absolute paths will start with
    a volume name and a colon. 
    
    :param path: The path to split
    :type path:  ``str``

    :return: the list of the path components
    :rtype:  ``list``
    """
    result = path.split('\\')
    return result


def path_split(path):
    """
    Returns a list with the components of a path description

    This function assumes that the path is in the native format for the platform: 
    Windows or Posix. Absolute paths can be identified by their first element
    (empty on Posix, a volume marker on Windows).

    :param path: The path to split
    :type path:  ``str``

    :return: the list of the path components
    :rtype:  ``list``
    """

    if any(platform.win32_ver()):
        return windows_split(path)
    return posix_split(path)


def remake_dir(*path):
    """
    Creates a directory at path, deleting what is already there
    
    :param path: The path of the directory to create
    :type path:  ``list`` of ``str``
    """
    result = os.path.join(*path)
    if os.path.exists(result):
        if (os.path.isdir(result)):
            shutil.rmtree(result)
        else:
            os.remove(result)
    os.mkdir(result)
    os.chmod(result,0o777)
    return result


def check_config_keys(config,entries):
    """
    Verifies that the dictionary has the given entries
    
    If any entry is missing this function will raise a KeyError
    
    :param config: The dictionary to check
    :type config:  ``dict``
    
    :param entries: The list of keys to check
    :type entries:  ``list``
    """
    for key in entries:
        if not key in config:
            raise KeyError('Config file is missing the %s entry.' % repr(key))


def entry_has_path(entry,*path):
    """
    Returns true if path is a chain of keys to a non-None value in entry
    
    For example, if path = ['a','b','c'], this function returns True if
    entry['a']['b']['c'] exists and is not None.
    
    :param entry: The dictionary entry
    :type entry:  ``dict``
    
    :param path: The chain of keys
    :type entry:  ``list``
    """
    root = entry
    for item in path:
        if not item in root:
            return False
        root = root[item]
    
    return not root is None


pass
#mark -
#mark FILES
def has_wildcards(text):
    """
    Returns true if text has (file) wildcard characters in it
    
    :param text: The text to check
    :type text:  ``str``
    
    :return: true if text has (file) wildcard characters in it
    :rtype:  ``bool``
    """
    return '*' in text or '?' in text or '[' in text or ']' in text


def expand_wildcards(root,pattern,category='all'):
    """
    Returns the list of files matching the given pattern
    
    Relative paths are computed relative to the root. The category specifies
    whether to match files, directories, or everything. All values are returned
    with POSIX paths.
    
    :param root: The root path for matching relative paths
    :type root:  ``str``
    
    :param pattern: The wildcard pattern to match
    :type pattern:  ``str``
    
    :param category: The type of file to match
    :type category:  One of 'file', 'directory', or 'all'
    
    :return: the list of files matching the given pattern
    :rtype:  ``list`` of ``str``
    """
    expand = pattern.split('/')
    
    # No absolute paths
    if len(expand) > 0 and expand[0] == '':
        expand = expand[1:]
    
    # Search for the file
    result = []
    path = os.path.join(root,*expand)
    for item in glob.glob(path):
        relpath = os.path.relpath(item,start=root)
        thepath = path_to_posix(relpath)
        if category == 'all':
            result.append(thepath)
        elif category == 'file' and not os.path.isdir(item):
            result.append(thepath)
        elif category == 'directory' and os.path.isdir(item):
            result.append(thepath)
    
    return result


def file_replace(path,data):
    """
    Modifies the file path, replacing all keys in data with their values.
    
    Example: if data = {'__NAME__':'Demo','__PCKG__':'org.libsdl.app'}, then
    this function replaces all instances of '__NAME__' in path with 'Demo', and all
    instances of '__PCKG__' with 'org.libsdl.app'.
    
    This function assumes that no key is a substring of any value. If that is not
    the case, the functionality is undefined.
    
    :param path: The file to modify
    'type path:  ``str``
    
    :param data: The key-value pairs for modification
    :type data:  ``dict``
    """
    contents = None
    with open(path) as file:
        contents = file.read()
        for key in data:
            contents = contents.replace(key,data[key])
    
    if contents:
        with open(path, 'w') as file:
            file.write(contents)
    else:
        raise FileNotFoundError('Could not locate %s.' % repr(path))


def file_replace_after(path,data):
    """
    Modifies the file path, replacing all keys in data with their values.
    
    Example: if data = {'__NAME__':'Demo','__PCKG__':'org.libsdl.app'}, then
    this function replaces all instances of '__NAME__' in path with 'Demo', and all
    instances of '__PCKG__' with 'org.libsdl.app'.
    
    This function assumes that no key is a substring of any value. If that is not
    the case, the functionality is undefined.
    
    :param path: The file to modify
    'type path:  ``str``
    
    :param data: The key-value pairs for modification
    :type data:  ``dict``
    """
    contents = None
    with open(path) as file:
        contents = file.read()
        for key in data:
            pos0 = 0
            pos1 = 0
            while pos0 != -1:
                pos0 = contents.find(key,pos1)
                if pos0 != -1:
                    pos0 += len(key)
                    pos1 = contents.find('\n',pos0)
                    if pos1 == -1:
                        pos1 = len(contents)
                    contents = contents[:pos0]+data[key]+contents[pos1:]
    
    if contents:
        with open(path, 'w') as file:
            file.write(contents)
    else:
        raise FileNotFoundError('Could not locate %s.' % repr(path))


def insert_filetree(filetree,files,target):
    """
    Inserts the files into a given file tree, for the given target
    
    A filetree is a recursive dictionary, not unlike a trie, specifying the path
    from a root directory. The leaves are tuples with the filename and target name.
    
    :param filetree: The filetree to modify
    :type filetree:  ``dict``
    
    :param files: The files to insert, expressed as POSIX paths
    :type files:  ``list`` of ``str``
    
    :param target: The target to annotate the file
    :type target:  ``str``
    """
    for file in files:
        path = file.split('/')
        prev = None
        curr = filetree
        for entry in path:
            if not entry in curr:
                curr[entry] = {}
            prev = curr
            curr = curr[entry]
        if prev:
            prev[path[-1]] = target


def directory_replace(path,contents,filter=None):
    """
    Recursive function to modify all files under path that satisfy filter
    
    This function starts at a path which represents a directory. It then descends
    this directory matching all files for which the function filter is true. For
    all such matches, it calls ``file_replace``, using contents as the modification
    dictionary.
    
    The filter function takes two arguments. The first is the current path in the
    recursive search. The second is the local name of a file relative to that path.
    If set to None, then all non-directory files are modified.
    
    The primary use of this function is to modify Android makefiles, as they are 
    arranged in a tree structure. In that case the filter would be the function
    
        lambda path, file: file == 'Android.mk'
    
    :param path: The path to the current (sub) root
    :type path:  ``str``
    
    :param filter: The filter function to identify files
    :type filter: (``str``,``str``) -> ``bool``
    
    :param data: The key-value pairs for modification
    :type data:  ``dict``
    """
    for item in os.listdir(path):
        abspath = os.path.join(path,item)
        if filter is None or filter(path,item):
            file_replace(abspath,contents)
        elif os.path.isdir(abspath):
            directory_replace(abspath,contents,filter)


def merge_copy(src,dst):
    """
    Copies src to dst, merging directories as necessary.
    
    Individual files will be overwritten, but directories will be merged. If any
    intermediate directories must be created to reach the destination, this is 
    function will do that as well.
    
    :param src: The file or directory to copy
    :type src: ``str``
    
    :param dst: The copy location
    :type src: ``str``
    """
    # Do nothing if src does not actual exist
    if not os.path.exists(src):
        return
    
    # Make sure the prefix of dst exists
    path = path_split(dst)
    if len(path) > 1:
        prefix = ''
        for pos in range(len(path)-1):
            prefix = os.path.join(prefix,path[pos])
            if not os.path.exists(prefix):
                os.mkdir(prefix)
    if os.path.isdir(src):
        if os.path.exists(dst):
            files = glob.glob(os.path.join(src,'*'))
            for item in files:
                suffix = os.path.split(item)[1]
                merge_copy(os.path.join(src,suffix),os.path.join(dst,suffix))
        else:
            shutil.copytree(src,dst)
    else:
        shutil.copyfile(src,dst)



pass
#mark -
#mark OTHER
def get_dev_platform():
    """
    Returns the development platform in use.
    
    We currently only support three development platforms: 'windows', 'macos', 
    and 'linux'.  For the most part, we do not need to identify our platform. 
    But in some cases (such as adding in external libraries), we do need to 
    know our platform.
    
    If the platform cannot be determined, this function will return 'unknown'.
    
    :return: The development platform in use.
    :rtype:  ``str``
    """
    if (any(platform.win32_ver())):
        return 'windows'
    if (any(platform.mac_ver())):
        return 'macos'
    try:
        platform.freedesktop_os_release()
        return 'linux'
    except:
        return 'unknown'


def group_parity(text,parens='()'):
    """
    Returns the parity for the given grouping
    
    A grouping is indicated by a pair of characters, like parentheses (default), brackets,
    or braces. If the pair are evenly matched, this returns 0. If we have an open pair,
    this returns a positive number. A negative number indicates that we have closed the
    pair too many times
    
    :param text: The text to check for parity
    :type text:  ``str``
    
    :param parens: The group delimiter pair
    :type parens:  ``str`` (of two characters)
    
    :return: The parity for the given grouping
    :rtype:  ``int``
    """
    result = 0
    for pos in range(len(text)):
        if text[pos] == parens[0]:
            result += 1
        elif text[pos] == parens[-1]:
            result -= 1
    return result


def merge_dict(d1, d2):
    """
    Returns a dictionary that it is a recursive merge of d1 and d2
    
    The resulting dictionary will contain all keys of d1 and d2. When a key
    appears in both d1 and d2, merge rules are defined as follows:
    
    - If both values are dicts, they are merged recursively
    - If both values are lists, the lists are concatenated, sorted, with dups removed
    - If both values are tuples, the tuples are concatenated, sorted, with dups removed
    - If only one value is a list or tuple, the other element is added with above rules
    - Otherwise, a sorted list is created with both elements (with dups removed)
    
    All values are (deep) copies.
    
    :param d1: The first dictionary
    :type d1:  ``dict``
    
    :param d2: The first dictionary
    :type d2:  ``dict``
    
    :return: The merge of dictionary d1 and d2
    :rtype:  ``dict``
    """
    import copy
    result = {}
    for key in d1:
        result[key] = copy.deepcopy(d1[key])
    
    for key in d2:
        if key in result:
            v1 = result[key]
            v2 = d2[key]
            if type(v1) == dict and type(v2) == dict:
                result[key] = merge_dict(v1,v2)
            elif type(v1) in [list,tuple] and type(v2) in [list,tuple]:
                v3 = list(v1)+list(v2)
                v3 = list(set(v3))
                try:
                    v3.sort()
                except:
                    pass
                result[key] = v3
            elif type(v1) in [list,tuple]:
                v3 = list(v1)+[v2]
                v3 = list(set(v3))
                try:
                    v3.sort()
                except:
                    pass
                result[key] = v3
            elif type(v2) in [list,tuple]:
                v3 = [v1]+list(v2)
                v3 = list(set(v3))
                try:
                    v3.sort()
                except:
                    pass
                result[key] = v3
            else:
                v3 = list(set((v1,copy.deepcopy(v2))))
                try:
                    v3.sort()
                except:
                    pass
                result[key] = v3
        else:
            result[key] = copy.deepcopy(d2[key])
    
    return result
