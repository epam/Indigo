import distutils.dir_util
import glob
import os
import shutil
import sys

from setuptools import setup  # type: ignore

CLASSIFIERS = """\
Development Status :: 5 - Production/Stable
Intended Audience :: Science/Research
Intended Audience :: Developers
License :: OSI Approved :: Apache Software License
Programming Language :: C
Programming Language :: C++
Programming Language :: Python
Programming Language :: Python :: 3
Programming Language :: Python :: 3.5
Programming Language :: Python :: 3.6
Programming Language :: Python :: 3.7
Programming Language :: Python :: 3.8
Programming Language :: Python :: 3.9
Programming Language :: Python :: 3.10
Programming Language :: Python :: Implementation :: CPython
Topic :: Software Development
Topic :: Scientific/Engineering :: Chemistry
Operating System :: Microsoft :: Windows
Operating System :: POSIX :: Linux
Operating System :: MacOS
"""

LONG_DESCRIPTION = "Indigo is a universal molecular toolkit that can be used for molecular fingerprinting, substructure search, and molecular visualization.\
Also capable of performing a molecular similarity search, it is 100% open source and provides enhanced stereochemistry support for end users, \
as well as a documented API for developers."

INDIGO_LIBS = None
PLATFORM_NAME = None

this_dir = os.path.dirname(os.path.abspath(__file__))
repo_root_dir = os.path.dirname(os.path.dirname(this_dir))
repo_dist_lib_dir = os.path.join(repo_root_dir, "dist", "lib")
indigo_python_directory = os.path.join(this_dir, "indigo")
indigo_native_libs_directory = os.path.join(indigo_python_directory, "lib")
if not os.path.exists(indigo_native_libs_directory):
    print(
        "No native libs found in {}, looking for them in {}".format(
            indigo_native_libs_directory, repo_dist_lib_dir
        )
    )
    if os.path.exists(repo_dist_lib_dir):
        print("Copying native libs from {}".format(repo_dist_lib_dir))
        shutil.copytree(repo_dist_lib_dir, indigo_native_libs_directory)

if sys.argv[1] == "bdist_wheel":
    for opt in sys.argv[2:]:
        if opt.startswith("--plat-name"):
            PLATFORM_NAME = opt.split("=")[1]
            if PLATFORM_NAME.startswith("macosx_10_7_intel"):
                INDIGO_LIBS = "lib/darwin-x86_64/*.dylib"
            elif PLATFORM_NAME.startswith("macosx_11_0_arm64"):
                INDIGO_LIBS = "lib/darwin-aarch64/*.dylib"
            elif PLATFORM_NAME == "manylinux2014_aarch64":
                INDIGO_LIBS = "lib/linux-aarch64/*.so"
            elif PLATFORM_NAME == "manylinux1_x86_64":
                INDIGO_LIBS = "lib/linux-x86_64/*.so"
            elif PLATFORM_NAME == "manylinux1_i686":
                INDIGO_LIBS = "lib/linux-i386/*.so"
            elif PLATFORM_NAME == "win_amd64":
                INDIGO_LIBS = "lib/windows-x86_64/*.dll"
            elif PLATFORM_NAME == "mingw":
                INDIGO_LIBS = "lib/windows-x86_64/*.dll"
            elif PLATFORM_NAME == "win32":
                INDIGO_LIBS = "lib/windows-i386/*.dll"
            break

    if not INDIGO_LIBS:
        raise ValueError(
            "Wrong --plat-name value! Should be one of: macosx_11_0_arm64, macosx_10_7_intel, manylinux1_x86_64, manylinux2014_aarch64, manylinux1_i686, win_amd64, win32"
        )

    if not glob.glob(os.path.join(indigo_python_directory, INDIGO_LIBS)):
        print(
            "No native libs found for platform {}, exiting".format(
                PLATFORM_NAME
            )
        )
        exit(0)
else:
    INDIGO_LIBS = "lib/**/*"

if os.path.exists("build"):
    distutils.dir_util.remove_tree("build")
if os.path.exists("indigo_chem.egg-info"):
    distutils.dir_util.remove_tree("indigo_chem.egg-info")

setup(
    name="epam.indigo",
    version="1.8.2",
    description="Indigo universal cheminformatics toolkit",
    author="EPAM Systems Life Science Department",
    author_email="lifescience.opensource@epam.com",
    maintainer="Mikhail Kviatkovskii",
    maintainer_email="Mikhail_Kviatkovskii@epam.com",
    packages=[
        "indigo",
    ],
    license="Apache-2.0",
    url="https://lifescience.opensource.epam.com/indigo/index.html",
    package_dir={"indigo": "indigo"},
    package_data={
        "indigo": [
            INDIGO_LIBS,
        ]
    },
    classifiers=[_f for _f in CLASSIFIERS.split("\n") if _f],
    platforms=["Windows", "Linux", "Mac OS-X"],
    long_description=LONG_DESCRIPTION,
    long_description_content_type="text/plain",
    project_urls={
        "Bug Tracker": "https://github.com/epam/indigo/issues",
        "Documentation": "https://lifescience.opensource.epam.com/indigo/api/index.html",
        "Source Code": "https://github.com/epam/indigo/",
    },
    download_url="https://pypi.org/project/epam.indigo",
    test_suite="tests",
    extras_require={
        "ml": ["scikit-learn", "torch", "pandas", "dgl", "tqdm", "click"],
        "notebooks": [
            "scikit-learn",
            "torch",
            "matplotlib",
            "pandas",
            "seaborn",
            "pandas",
            "bokeh",
            "lightgbm",
            "optuna",
        ],
    },
)
