from setuptools import setup

packages = ["indigo_service"]

package_data = {"": ["*"]}

install_requires = [
    "aiofiles>=0.7.0,<0.8.0",
    "epam.indigo>=1.4.3,<2.0.0",
    "fastapi>=0.63.0,<0.64.0",
    "uvicorn[standard]>=0.13.3,<0.14.0",
]

entry_points = {
    "console_scripts": [
        "indigo_service = indigo_service.indigo_http:run_debug"
    ]
}

setup_kwargs = {
    "name": "indigo-service",
    "version": "0.0.1",
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
    "python_requires": ">=3.7,<4.0",
}


setup(**setup_kwargs)
