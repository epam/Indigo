import distutils.dir_util
import os
import sys

from setuptools import setup

CLASSIFIERS = """\
Development Status :: 4 - Beta
Intended Audience :: Science/Research
Intended Audience :: Developers
License :: OSI Approved :: Apache Software License
Programming Language :: C
Programming Language :: C++
Programming Language :: Python
Programming Language :: Python :: 2
Programming Language :: Python :: 2.7
Programming Language :: Python :: 3
Programming Language :: Python :: 3.0
Programming Language :: Python :: 3.1
Programming Language :: Python :: 3.2
Programming Language :: Python :: 3.3
Programming Language :: Python :: 3.5
Programming Language :: Python :: 3.6
Programming Language :: Python :: 3.7
Programming Language :: Python :: Implementation :: CPython
Topic :: Software Development
Topic :: Scientific/Engineering :: Chemistry
Operating System :: Microsoft :: Windows
Operating System :: POSIX :: Linux
Operating System :: MacOS
"""


INDIGO_LIBS = None

if sys.argv[1] == 'bdist_wheel':
    for opt in sys.argv[2:]:
        if opt.startswith('--plat-name'):
            name = opt.split('=')[1]
            if name.startswith('macosx_10_7'):
                INDIGO_LIBS = 'lib/Mac/10.7/*.dylib'
            elif name == 'manylinux1_x86_64':
                INDIGO_LIBS = 'lib/Linux/x64/*.so'
            elif name == 'manylinux1_i686':
                INDIGO_LIBS = 'lib/Linux/x86/*.so'
            elif name == 'win_amd64':
                INDIGO_LIBS = 'lib/Win/x64/*.dll'
            elif name == 'win32':
                INDIGO_LIBS = 'lib/Win/x86/*.dll'
            break

if not INDIGO_LIBS:
    raise ValueError('Wrong --plat-name value! Should be one of: macosx_10_7, manylinux1_x86_64, manylinux1_i686, win_amd64, win32')

if os.path.exists('build'):
    distutils.dir_util.remove_tree('build')
if os.path.exists('indigo_chem.egg-info'):
    distutils.dir_util.remove_tree('indigo_chem.egg-info')

setup(
    name='epam.indigo',
    version='1.4.0-beta',
    description='Indigo universal cheminformatics toolkit',
    author='EPAM Systems Lifescience Department',
    author_email='lifescience.opensource@epam.com',
    maintainer='Mikhail Kviatkovskii',
    maintainer_email='Mikhail_Kviatkovskii@epam.com',
    packages=['indigo', ],
    license="Apache-2.0",
    url="http://github.com/epam/indigo.git",
    package_dir={'indigo': 'indigo'},
    package_data={'indigo': [INDIGO_LIBS, ]},
    classifiers=[_f for _f in CLASSIFIERS.split('\n') if _f],
    platforms=["Windows", "Linux", "Mac OS-X"],
)
