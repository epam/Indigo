import sys
import os
import platform
from env_indigo import isIronPython, isJython

if sys.version_info > (3, 0):
    from bistring3 import BitString
else:
    from bistring1 import BitString

HASH_SIZE = 32

if isIronPython():
    import clr

    clr.AddReference("System")
    clr.AddReference("System.Runtime.Extensions")
    clr.AddReference("System.Runtime.InteropServices.RuntimeInformation")
    import System
    import System.Environment
    import System.Runtime.InteropServices.RuntimeInformation
    dotnet_framework = System.Runtime.InteropServices.RuntimeInformation.FrameworkDescription
    if dotnet_framework.startswith('.NET Core'):
        clr.AddReferenceToFileAndPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'dotnet', 'System.Drawing.Common.dll'))
        os_version = System.Environment.OSVersion.ToString()

        if os_version.startswith('Unix'):
            clr.AddReferenceToFileAndPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'dotnet', 'System.Drawing.Common.unix.dll'))
        elif os_version.startswith('Microsoft Windows'):
            clr.AddReferenceToFileAndPath(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'dotnet', 'System.Drawing.Common.win.dll'))
        else:
            raise SystemError("Unsupported OS: " + os_version)

    clr.AddReference("System.Drawing")
    from System.Drawing import Bitmap, Graphics, Rectangle, Drawing2D
elif isJython():
    from java.lang import System
    from java.awt.image import BufferedImage
    from javax.imageio import ImageIO
    from java.io import File
    from java.awt import RenderingHints, Image
else:
    try:
        import Image
    except ImportError:
        from PIL import Image


class RenderingTestException(Exception):
    pass


class ImageHash(object):
    def __init__(self, path, size=HASH_SIZE):
        self.image_path = path
        self.hash_size = size

    def getBitString(self, pixels):
        avg = sum(pixels) / len(pixels)
        diff = []
        for pixel in pixels:
            value = 1 if pixel > avg else 0
            diff.append(str(value))
        bits = BitString(bin="".join(diff))
        return bits

    def average_hash_and_sizes(self):
        height = 0
        width = 0
        if isJython():
            image = ImageIO.read(File(self.image_path))
            height = image.getHeight()
            width = image.getWidth()
            newImage = BufferedImage(self.hash_size, self.hash_size, BufferedImage.TYPE_INT_ARGB)
            g = newImage.createGraphics()
            g.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_BICUBIC)
            g.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY)
            g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON)
            g.drawImage(image, 0, 0, self.hash_size, self.hash_size, None)
            g.dispose()
            allchannelpixels = [[], [], [], []]
            for i in range(self.hash_size):
                for j in range(self.hash_size):
                    pixel = int(newImage.getRGB(i, j))
                    allchannelpixels[0].append((pixel >> 16) & 0xFF)
                    allchannelpixels[1].append((pixel >> 8) & 0xFF)
                    allchannelpixels[2].append((pixel) & 0xFF)
                    allchannelpixels[3].append((pixel >> 24) & 0xFF)
        elif isIronPython():
            srcImage = Bitmap(self.image_path)
            height = srcImage.Height
            width = srcImage.Width
            newImage = Bitmap(self.hash_size, self.hash_size)
            gr = Graphics.FromImage(newImage)
            gr.SmoothingMode = Drawing2D.SmoothingMode.AntiAlias
            gr.InterpolationMode = Drawing2D.InterpolationMode.HighQualityBicubic
            gr.PixelOffsetMode = Drawing2D.PixelOffsetMode.HighQuality
            gr.DrawImage(srcImage, Rectangle(0, 0, self.hash_size, self.hash_size))
            allchannelpixels = [[], [], [], []]
            for i in range(self.hash_size):
                for j in range(self.hash_size):
                    pixel = newImage.GetPixel(i, j)
                    allchannelpixels[0].append(int(pixel.R))
                    allchannelpixels[1].append(int(pixel.G))
                    allchannelpixels[2].append(int(pixel.B))
                    allchannelpixels[3].append(int(pixel.A))
        else:
            self.image = Image.open(self.image_path)
            width, height = self.image.size
            image = self.image.resize((self.hash_size, self.hash_size), Image.ANTIALIAS)
            # image.show()
            allchannelpixels = [list(channel.getdata()) for channel in image.split()]

        bits = []
        for pixels in allchannelpixels:
            bits.append(self.getBitString(pixels))
        return bits, width, height


def imageDiff(imp1, imp2):
    imh1, im1_width, im1_height = ImageHash(imp1, HASH_SIZE).average_hash_and_sizes()
    imh2, im2_width, im2_height = ImageHash(imp2, HASH_SIZE).average_hash_and_sizes()
    if (abs((float(im1_width) / float(im2_width)) - 1.0) > 0.1) or (abs((float(im1_height) / float(im2_height)) - 1.0) > 0.1):
        raise RenderingTestException("Images have different sizes: %sx%s (ref) and %sx%s (out)" % (im1_width, im1_height, im2_width, im2_height))
    if len(imh1) != len(imh2):
        raise RenderingTestException("Images have different channels count: %s (ref) and %s (out)" % (len(imh1), len(imh2)))
    results = []
    for i in range(len(imh1)):
        results.append((imh1[i] ^ imh2[i]).bin.count("1"))
    return results


# def checkSvgSimilarity(filename):
#     def blankId(n):
#         return 'id=""'
#
#     def roundFloat(n):
#         try:
#             return str(int(round(float(n.group(0)))))
#         except TypeError, e:
#             return n.group(0)
#
#     f = open('ref/%s' % filename)
#     ref = f.read()
#     f.close()
#     f = open('out/%s' % filename)
#     out = f.read()
#     f.close()
#
#     pattern = re.compile('([0-9]+\.[0-9]+)')
#     ref = pattern.sub(roundFloat, ref)
#     out = pattern.sub(roundFloat, out)
#
#     pattern = re.compile('id=\"(.+)\"')
#     ref = pattern.sub(blankId, ref)
#     out = pattern.sub(blankId, out)
#
#     value = difflib.SequenceMatcher(None, ref, out).ratio()
#     if value >= 0.75:
#         print '%s rendering status: OK' % filename
#     else:
#         print '%s rendering status: Problem: SVG similarity is %s' % (filename, round(value, 2))


def checkBitmapSimilarity(filename, ref_filename):
    if ref_filename is None:
        ref_filename = filename
    try:
        if os.name == 'nt':
            system = 'win'
        elif os.name == 'posix':
            if not platform.mac_ver()[0]:
                system = 'linux'
            else:
                system = 'mac'
        elif os.name == 'java':
            osName = System.getProperty("os.name")
            if osName.find("Windows") != -1:
                system = 'win'
            elif osName.find('Linux') != -1:
                system = 'linux'
            elif osName.find('Mac OS') != -1:
                system = 'mac'
            else:
                raise RenderingTestException("No reference images for this operating system: {0}".format(osName))
        else:
            raise RenderingTestException("No reference images for this operating system: {0}".format(os.name))
        dirname = os.path.normpath(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', 'tests', 'rendering')))
        results = imageDiff('%s/ref/%s/%s' % (dirname, system, ref_filename), '%s/out/%s' % (dirname, filename))
    except RenderingTestException as e:
        return '%s rendering status: Problem: %s' % (filename, str(e))

    channels = ['red', 'green', 'blue', 'alpha']
    for i, result in enumerate(results):
        if result > (HASH_SIZE ** 2) * 0.1:
            return '%s rendering status: Problem: PNG similarity is %s for %s channel' % (filename, round(1 - (result / float(HASH_SIZE ** 2)), 2), channels[i])

    return '%s rendering status: OK' % filename


def checkImageSimilarity(filename, ref_filename=None):
    if filename.endswith('.svg'):
        # checkSvgSimilarity(filename)
        return ''
    else:
        return checkBitmapSimilarity(filename, ref_filename)
