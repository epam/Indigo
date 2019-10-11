import distutils.dir_util
import os
import sys

from setuptools import setup

INDIGO_LIBS = None

if sys.argv[1] == 'bdist_wheel':
    for opt in sys.argv[1:]:
        if opt.startswith('--plat-name'):
            name = opt.split('=')[1]
            INDIGO_LIBS = 'lib/Mac/10.7/*.dylib' if name.startswith('macosx_10_7') else \
                'lib/Linux/x64/*.so' if name == 'manylinux1_x86_64' else \
                    'lib/Linux/x86/*.so' if name == 'manylinux1_i686' else \
                        'lib/Win/x64/*.dll' if name == 'win_amd64' else \
                            'lib/Win/x86/*.dll' if name == 'win32' else None
            break

if not INDIGO_LIBS:
    raise ValueError('Wrong --plat-name value! Should be one of: macosx_10_7, manylinux1_x86_64, manylinux1_i686, win_amd64, win32')

if os.path.exists('build'):
    distutils.dir_util.remove_tree('build')
if os.path.exists('indigo_chem.egg-info'):
    distutils.dir_util.remove_tree('indigo_chem.egg-info')

setup(
    name='indigo-chem',
    version='1.4.0-beta',
    description='Indigo universal cheminformatics toolkit',
    author='EPAM Systems Lifescience Department',
    author_email='lifescience.opensource@epam.com',
    maintainer='Mikhail Kviatkovskii',
    maintainer_email='Mikhail_Kviatkovskii@epam.com',
    packages=['indigo', 'indigo_inchi', 'indigo_renderer', 'bingo'],
    license="Apache-2.0",
    url="http://github.com/epam/indigo",
    package_dir={'indigo': 'indigo'},
    package_data={'indigo': [INDIGO_LIBS, ]},
)
