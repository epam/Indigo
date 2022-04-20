from setuptools import setup  # type: ignore

CLASSIFIERS = """\
Development Status :: 5 - Production/Stable
Intended Audience :: Science/Research
Intended Audience :: Developers
License :: OSI Approved :: Apache Software License
Programming Language :: Python
Programming Language :: Python :: 3.7
Programming Language :: Python :: 3.8
Programming Language :: Python :: 3.9
Programming Language :: Python :: 3.10
Programming Language :: Python :: Implementation :: CPython
Topic :: Scientific/Engineering :: Chemistry
Topic :: Database
Topic :: Database :: Database Engines/Servers
Operating System :: Microsoft :: Windows
Operating System :: POSIX :: Linux
Operating System :: MacOS
"""

setup(
    name="bingo_elastic",
    version="1.7.0-beta",
    description="Cartridge that provides fast, scalable, and efficient storage and searching solution for chemical information using Elasticsearch",
    author="Ruslan Khyurri",
    author_email="ruslan_khyurri@epam.com",
    license="Apache-2.0",
    url="https://github.com/epam/Indigo/tree/master/bingo/bingo-elastic/python",
    project_urls={
        "Bug Tracker": "https://github.com/epam/indigo/issues",
        "Documentation": "https://github.com/epam/Indigo/tree/master/bingo/bingo-elastic/python",
        "Source Code": "https://github.com/epam/indigo/",
    },
    download_url="https://pypi.org/project/bingo_elastic",
    python_requires=">=3.7",
    packages=["bingo_elastic", "bingo_elastic.model"],
    install_requires=["epam.indigo==1.6.1", "elasticsearch==7.16.2"],
    extras_require={
        "async": ["elasticsearch[async]==7.16.2"],
        "dev": [
            "pylint",
            "pytest",
            "wheel",
            "black",
            "pytest-asyncio",
            "mypy",
        ],
    },
    classifiers=[_f for _f in CLASSIFIERS.split("\n") if _f],
)
