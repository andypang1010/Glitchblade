"""
Script to generate CUGL icons

Icon generation is always annoying, because each platform has its own specific format.
This script automates this process by generating the standard formats for macOS, iOS,
Android and Windows.

To work it must be given a 1024x1024 PNG file for the icon. The icon may have transparency,
but this transparency will be removed on all mobile platforms (where it is not allowed).
On those platforms the image will be composited on top of a background color. By default
this background color is white, but it can be specified with a custom attribute.

The transparent attribute will only apply the backing color on mobile platforms. Otherwise
it will be applied on both desktop and mobile platforms. The rounded attribute will apply
rounded corners on the desktop icons.  Both it and transparent cannot be true; transparent
will take priority.

Author: Walker White
Date: February 22, 2022
"""
from PIL import Image
import traceback
import os, os.path
import shutil
import math
import json
import argparse


#mark COLOR FUNCTIONS

def rgb_to_web(rgb):
    """
    Returns a web color equivalent to the given rgb tuple
    
    Web colors are 7 characters starting with a # and followed by a six character
    hexadecimal string (2 characters per channel).
    
    :param rgb: The rgb color value
    :type rgb:  3-element tuple of ints 0..255
    """
    r = hex(rgb[0]).lower()[-2:]
    g = hex(rgb[1]).lower()[-2:]
    b = hex(rgb[2]).lower()[-2:]
    return "#"+r+g+b


def web_to_rgb(web):
    """
    Returns an rgb tuple for the given web color.
    
    The web color is a string that may or may not start with an #. It should have
    6 hexadecimal digits (2 characters per channel). The value returned is a 3-element
    tuple of ints in the range 0..255.
    
    :param web: The web color
    :type web:  ``str``
    """
    try:
        if web[0] == '#':
            web = web[1:]
        if len(web) < 6:
            web = web + ('f'*(6-len(web)))
        web = web.lower()
        r = int(web[0:2],16)
        g = int(web[2:4],16)
        b = int(web[4:6],16)
        return (r,g,b)
    except:
        print(traceback.format_exc()[:-1])
        return (255,255,255)

pass
#mark -
#mark IMAGE RESAMPLING

def rdrect_image(image):
    """
    Returns a copy of image with rounded corners
    
    The corners of the image will be erased to turn the image into a
    rounded rectangle. This method is intended to be applied to an image
    with no transparency and can have unexpected results if the image has
    transparency.
    
    The radius of each of the rounded rectangle corners is 10% of the
    minium image dimension.
    
    :param image: The image to copy
    :type image:  PIL Image object
    """
    radius = min(*image.size)/10
    data = image.getdata()
    result = []
    
    w = image.size[0]
    h = image.size[0]
    for y in range(h):
        for x in range(w):
            p = data[x+y*w]
            a = rdrect_alpha(x,y,w,h,radius)
            a = round(a*255)
            p = (p[0],p[1],p[2],a)
            result.append(p)
    copy = Image.new('RGBA',(w,h),0)
    copy.putdata(result)
    return copy


def rdrect_alpha(x,y,w,h,r):
    """
    Computes the alpha percentage at (x,y) in the rounded rectangle (w,h,r).
    
    This function is used to give antialiased edges to a rounded rectangle. The
    rectangle has origin (0,0) and size (w,h). Each corner has radius r.
    
    :param x: The x-coordinate to test
    :type x:  ``int`` or ``float``
    
    :param y: The y-coordinate to test
    :type y:  ``int`` or ``float``
    
    :param w: The rectangle width
    :type w:  ``int`` or ``float``
    
    :param h: The rectangle height
    :type h:  ``int`` or ``float``
    
    :param r: The corner radius
    :type r:  ``int`` or ``float``
    """
    if (x < 0 or x > w):
        return 0
    elif (y < 0 or y > h):
        return 0
    elif (x <= r and y <= r):
        far  = math.sqrt((x-r)*(x-r)+(y-r)*(y-r))
        near = math.sqrt((x+1-r)*(x+1-r)+(y+1-r)*(y+1-r))
    elif (x < r and y > h-r):
        far  = math.sqrt((x-r)*(x-r)+(y-h+r)*(y-h+r))
        near = math.sqrt((x+1-r)*(x+1-r)+(y-1-h+r)*(y-1-h+r))
    elif (x > w-r and y > h-r):
        far  = math.sqrt((x-w+r)*(x-w+r)+(y-h+r)*(y-h+r))
        near = math.sqrt((x-1-w+r)*(x-1-w+r)+(y-1-h+r)*(y-1-h+r))
    elif (x > w-r and y < r):
        far  = math.sqrt((x-w+r)*(x-w+r)+(y-r)*(y-r))
        near = math.sqrt((x-1-w+r)*(x-1-w+r)+(y+1-r)*(y+1-r))
    else:
        far  = 0
        near = 0
    
    if near >= r:
        return 0
    elif far <= r:
        return 1
    else: # near < r < far
        return (r-near)/math.sqrt(2)


def circle_image(image):
    """
    Returns a copy of image inscribed inside of a circle.
    
    The circle is the largest one the inscribes the image rectangle. Hence its diameter
    is the minimum of the image width and height.
    
    :param image: The image to copy
    :type image:  PIL Image object
    """
    data = image.getdata()
    result = []
    
    w = image.size[0]
    h = image.size[0]
    for y in range(h):
        for x in range(w):
            p = data[x+y*w]
            a = circle_alpha(x,y,w,h)
            a = round(a*255)
            p = (p[0],p[1],p[2],a)
            result.append(p)
    copy = Image.new('RGBA',(w,h),0)
    copy.putdata(result)
    return copy


def circle_alpha(x,y,w,h):
    """
    Computes the alpha percentage at (x,y) in the circle inscribing the rectangle (w,h).
    
    This function is used to give antialiased edges to a circle. The circle has center
    (w/2,h/2) and its diameter is the minimum of the w and h.
    
    :param x: The x-coordinate to test
    :type x:  ``int`` or ``float``
    
    :param y: The y-coordinate to test
    :type y:  ``int`` or ``float``
    
    :param w: The bounding width
    :type w:  ``int`` or ``float``
    
    :param h: The bounding height
    :type h:  ``int`` or ``float``
    """
    c = (w/2,h/2)
    r = min(*c)
    far = math.sqrt((x-c[0])*(x-c[0])+(y-c[1])*(y-c[1]))
    
    if (x < 0 or x > w):
        return 0
    elif (y < 0 or y > h):
        return 0
    elif (x <= c[0] and y <= c[1]):
        near = math.sqrt((x+1-c[0])*(x+1-c[0])+(y+1-c[1])*(y+1-c[1]))
    elif (x <= c[0] and y >= c[1]):
        near = math.sqrt((x+1-c[0])*(x+1-c[0])+(y-1-c[1])*(y-1-c[1]))
    elif (x >= c[0] and y >= c[1]):
        near = math.sqrt((x-1-c[0])*(x-1-c[0])+(y-1-c[1])*(y-1-c[1]))
    elif (x >= c[0] and y <= c[1]):
        near = math.sqrt((x-1-c[0])*(x-1-c[0])+(y+1-c[1])*(y+1-c[1]))
    else:
        far  = 0
        near = 0
    
    if near >= r:
        return 0
    elif far <= r:
        return 1
    else: # near < r < far
        return (r-near)/math.sqrt(2)


def blend_image(image,color):
    """
    Returns a copy of the image blended with the given background color.
    
    :param image: The image to copy
    :type image:  PIL Image object
    """
    if image is None:
        return None
    
    data = image.getdata()
    result = []
    if image.mode == 'RGB':
        for pix in data:
            result.append((pix[0],pix[1],pix[2],255))
    else:
        for pix in data:
            if pix[3] != 255:
                a = pix[3]/255.0
                r = min(int(pix[0]*a+color[0]*(1-a)),255)
                g = min(int(pix[1]*a+color[1]*(1-a)),255)
                b = min(int(pix[2]*a+color[2]*(1-a)),255)
                pix = (r,g,b,255)
            result.append(pix)
    
    copy = Image.new('RGBA',image.size,0)
    copy.putdata(result)
    return copy


def float_image(image):
    """
    Returns a copy of the image resized for Android dynamic icons.
    
    This method requires that that image is be square.  It resizes the image to be
    twice the size of the highest resolution icon (for better downscaling), and adds
    the empty padding that dynamic icons require.
    
    :param image: The image to copy
    :type image:  PIL Image object
    """
    dold = 640
    dnew = 864
    copy = image.resize((dold,dold))
    data = copy.getdata()
    next = Image.new('RGBA',(dnew,dnew),0)
    
    result = []
    for y in range(dnew):
        for x in range(dnew):
            diff = (dnew-dold)//2
            if x < diff or y < diff or x >= diff+dold or y >= diff+dold:
                p = (0,0,0,0)
            else:
                p = data[(x-diff)+(y-diff)*dold]
            result.append(p)
    
    next.putdata(result)
    return next


pass
#mark -
#mark ICON FACTORY

class IconMaker(object):
    """
    A class for generating application icons.
    
    The class currently supports macOS, iOS, Android, and Windows.
    
    It works by specifying a foreground image and a background color. If the foreground
    image is opaque, the color has no effect. But images with transparency require
    a background color on mobile devices.
    """
    
    def __init__(self,file,color=None,transparent=False,rounded=False):
        """
        Initializes the icon maker with the given values
        
        If the background color is not specified, the maker will use white as the
        background color.
        
        :param file: The file name of the foreground image
        :type file:  ``str``
        
        :param color: The background color
        :type color:  ``str`` or ``None``
        
        :param transparent: Whether to preserve transparency on desktop icons
        :type transparent:  ``bool``
        
        :param rounded: Whether to round the corners on desktop icons (like mobile)
        :type rounded:  ``bool``
        """
        self.acquire(file)
        self._color = web_to_rgb(color)
        self._transparent = transparent
        self._rounded = rounded
    
    def acquire(self,file):
        """
        Loads the file into a PIL image
        
        If this fails, attributes image and name will be None.
        
        :param file: The file name of the foreground image
        :type file:  ``str``
        """
        try:
            self._image = Image.open(file)
            if not self._image:
                print('Cannot open file "%s"' % file)
            elif not self._image.mode in ['RGBA','RGB']:
                print('Image has invalid format: '+self._image.mode)
                self._image = None
            elif self._image.size != (1024,1024):
                print('Image has invalid size: '+repr(self._image.size))
                self._image = None
            self._name = os.path.splitext(os.path.split(file)[1])[0]
        except:
            print(traceback.format_exc()[:-1])
            self._image = None
            self._name =  None
    
    def generate(self,root):
        """
        Generates the icons for all major platforms.
        
        The icons will be placed relative to the given root directory. Each platform will
        get a subfolder for its name (e.g. iOS, macOS, android, windows). See the platform
        specific generation methods for information about further subfolders.
        
        :param root: The root directory for the application icons
        :type root:  ``str``
        """
        if self._image is None:
            print("Could not generate icons")
            return
        
        self.gen_macos(os.path.join(root,'iOS'))
        self.gen_ios(os.path.join(root,'macOS'))
        self.gen_android(os.path.join(root,'android'))
        self.gen_windows(os.path.join(root,'windows'))
    
    def gen_macos(self,root):
        """
        Generates the icons for macOS
        
        The icons will be placed in the 'Mac.xcassets/AppIcon.appiconset' subdirectory
        of the root directory. To add icons directly to an XCode project, the root should
        be the Resources folder of your XCode project.
        
        :param root: The root directory for the application icons
        :type root:  ``str``
        """
        if self._image is None:
            print("Could not generate icons")
            return
        
        path = root
        subdirs = ['Mac.xcassets','AppIcon.appiconset']
        for item in subdirs:
            path = os.path.join(path,item)
            if (not os.path.isdir(path)):
                os.mkdir(path)
        
        if self._transparent:
            small = self._image
            large = self._image
        else:
            small = blend_image(self._image,self._color)
            large = small
        
        if self._rounded:
            small = blend_image(small,self._color)
            large = rdrect_image(small)
        
        size = ((16,1),(16,2),(32,1),(32,2),(128,1),(128,2),(256,1),(256,2),(512,1),(512,2))
        data = []
        for s in size:
            info = {}
            info['filename'] = self._name
            info['idiom'] = 'mac'
            info['size'] = str(s[0])+'x'+str(s[0])
            info['scale'] = str(s[1])+'x'
            if s[1] == 1:
                suffix = '_' +info['size'] +'.png'
            else:
                suffix = '_' +info['size'] +'@'+info['scale']+'.png'
            info['filename'] = info['filename']+suffix
            data.append(info)
            
            d = s[0]*s[1]
            if d > 64:
                copy = large.resize((d,d))
            else:
                copy = small.resize((d,d))
            copy.save(os.path.join(path,info['filename']),'PNG')
        
        contents = {'images':data}
        contents['info'] = { 'version' : 1, 'author' : 'xcode' }
        with open(os.path.join(path,'Contents.json'),'w') as file:
            file.write(json.dumps(contents,indent=2))
    
    def gen_ios(self,root):
        """
        Generates the icons for iOS
        
        The icons will be placed in the 'iOS.xcassets/AppIcon.appiconset' subdirectory
        of the root directory. To add icons directly to an XCode project, the root should
        be the Resources folder of your XCode project.
        
        :param root: The root directory for the application icons
        :type root:  ``str``
        """
        if self._image is None:
            print("Could not generate icons")
            return
        
        path = root
        subdirs = ['iOS.xcassets','AppIcon.appiconset']
        for item in subdirs:
            path = os.path.join(path,item)
            if (not os.path.isdir(path)):
                os.mkdir(path)
        
        # Updated to XCode 16.2
        formats = (
                    (20,1),(20,2),(20,3),(29,1),(29,2),(29,3),(38,1),(38,2),(38,3),
                    (40,1,20,2),(40,2,20,4),(40,3,60,2),(60,2),(60,3),(64,2),(64,3),
                    (68,2),(76,1,38,2),(76,2,38,4),(83.5,2),(1024,1)
                  )
        image = blend_image(self._image,self._color)
        
        data = []
        made = {''}
        for s in formats:
            info = {}
            dim   = s[0]
            scale = s[1]
            info['filename'] = self._name
            info['idiom'] = "universal"
            info['platform'] = "ios"
            info['size'] = str(dim)+'x'+str(dim)
            if s[1] != '1':
                info['scale'] = str(scale)+'x'

            if len(s) == 4:
                dim   = s[2]
                scale = s[3]
                
            if scale == 1:
                suffix = '-'+str(dim) +'.png'
            else:
                suffix = '-'+ str(dim) + '@'+str(scale)+'x.png'
            info['filename'] =  info['filename'] + suffix
            data.append(info)
            
            if (not info['filename'] in made):
                made.add(info['filename'])
                d = int(dim*scale)
                copy = image.resize((d,d))
                copy.save(os.path.join(path,info['filename']),'PNG')
        
        contents = {'images':data}
        contents['info'] = { 'version' : 1, 'author' : 'xcode' }
        with open(os.path.join(path,'Contents.json'),'w') as file:
            file.write(json.dumps(contents,indent=2))
    
    def gen_android(self,root):
        """
        Generates the icons for Android
        
        The icons will be placed in the given directory. If the directory is the resource
        directory for an existing Android project, it will copy the images into the
        appropriate folders. Otherwise, it will generate the folders so that you can
        copy them manually.
        
        :param root: The root directory for the application icons
        :type root:  ``str``
        """
        if self._image is None:
            print("Could not generate icons")
            return
        
        resdir  = os.path.join(root,'res')
        if not os.path.isdir(resdir):
            resdir = root
        
        subdirs = ['mipmap-mdpi','mipmap-hdpi','mipmap-xhdpi','mipmap-xxhdpi','mipmap-xxxhdpi','values']
        for d in subdirs:
            path = os.path.join(resdir,d)
            if (not os.path.isdir(path)):
                os.mkdir(path)
        
        path = os.path.join(resdir,'values','ic_launcher_background.xml')
        xml = '<?xml version="1.0" encoding="utf-8"?>\n<resources>\n    <color name="ic_launcher_background">%s</color>\n</resources>\n'
        with open(path,'w') as file:
            file.write(xml % rgb_to_web(self._color))
        
        image = blend_image(self._image,self._color)
        round = circle_image(image)
        float = float_image(self._image)
        
        # Save play store version
        image.save(os.path.join(root,'ic_launcher-playstore.png'),'PNG')
        
        sizes =  ((48,108),(72,162),(96,216),(144,324),(192,432))
        for p in range(len(sizes)):
            path = os.path.join(resdir,subdirs[p],'ic_launcher')
            d1 = sizes[p][0]
            d2 = sizes[p][1]
            image.resize((d1,d1)).save(path+'.png','PNG')
            round.resize((d1,d1)).save(path+'_round.png','PNG')
            float.resize((d2,d2)).save(path+'_foreground.png','PNG')
    
    def gen_windows(self,root):
        """
        Generates the icons for Window
        
        This will generate a single .ICO file in the given directory. This file
        should be copied to the appropriate location in your Visual Studio Project.
        
        Windows ignores the transparency setting as ICO files do not support
        antialiasing very well.
        
        :param root: The root directory for the application icons
        :type root:  ``str``
        """
        if self._image is None:
            print("Could not generate icons")
            return
        
        if self._rounded:
            image = blend_image(self._image,self._color)
            image = rdrect_image(image)
        else:
            image = blend_image(self._image,self._color)
        
        image.save(os.path.join(root,'icon1.ico'),'ICO')
