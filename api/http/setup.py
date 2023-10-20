from setuptools import setup  # type: ignore

packages = ["indigo_service"]

package_data = {"": ["*"]}

install_requires = [
    "aiofiles>=0.7.0",
    "epam.indigo>=1.4.3",
    "fastapi>=0.63.0",
    "uvicorn[standard]>=0.13.3",
]

entry_points = {
    "console_scripts": [
        "indigo_service = indigo_service.indigo_http:run_debug"
    ]
}

setup_kwargs = {
    "name": "indigo-service",
    "version": "1.14.0.rc7",
    "description": "",
    "long_description": None,
    "author": "Ruslan Khyurri",
    "author_email": "ruslan_khyurri@epam.com",
    "maintainer": None,
    "maintainer_email": None,
    "url": None,
    "packages": packages,
    "package_data": package_data,
    "install_requires": install_requires,
    "entry_points": entry_points,
    "python_requires": ">=3.7",
}


setup(**setup_kwargs)
