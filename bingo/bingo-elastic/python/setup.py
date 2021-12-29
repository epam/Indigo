from setuptools import setup

setup(
    name="bingo_elastic",
    version="1.6.1",
    description="Bingo chemical cartridge for Elasticsearch",
    authors=["Ruslan Khyurri <ruslan_khyurri@epam.com>"],
    license="Apache License 2.0",
    python_requires=">=3.7",
    packages=[
        "bingo_elastic",
    ],
    install_requires=[
        "epam.indigo==1.6.1",
        "elasticsearch==7.16.2"
    ],
)
