import configparser
import os
import re
import sys
import xml.etree.cElementTree as ElementTree


INDIGO_PATH = os.path.normpath(
    os.path.join(os.path.abspath(os.path.dirname(__file__)), os.path.pardir)
)


def update_pom_version(pom_file, new_version_tuple):
    version_prefix, version_suffix, commits, _ = new_version_tuple
    new_version = version_prefix
    if version_suffix:
        new_version += "-{}".format(version_suffix)
    if commits:
        dot_symbol = "." if not version_suffix else ""
        new_version += "{}{}".format(dot_symbol, commits)

    tree = ElementTree.parse(pom_file)
    ElementTree.register_namespace("", "http://maven.apache.org/POM/4.0.0")
    root = tree.getroot()
    version_changed = False
    for child in root:
        if child.tag.endswith("version"):
            old_version = child.text
            if old_version != new_version:
                print(
                    "Updating Indigo version from {0} to {1} in {2}...".format(
                        old_version, new_version, pom_file
                    )
                )
                child.text = new_version
                version_changed = True
            else:
                print(
                    "Indigo version in {0} remains {1}...".format(pom_file, new_version)
                )
            break
    if version_changed:
        tree.write(pom_file)


def update_csproj_version(csproj_file, new_version_tuple):
    version_prefix, version_suffix, commits, _ = new_version_tuple
    new_version = version_prefix
    if version_suffix:
        new_version += "-{}".format(version_suffix)
    if commits:
        new_version += ".{}".format(commits)

    tree = ElementTree.parse(csproj_file)
    root = tree.getroot()
    version_changed = False
    for child in root:
        if child.tag == "PropertyGroup":
            for cc in child:
                if cc.tag == "Version":
                    old_version = cc.text
                    if old_version != new_version:
                        print(
                            "Updating Indigo version from {0} to {1} in {2}...".format(
                                old_version, new_version, csproj_file
                            )
                        )
                        cc.text = new_version
                        version_changed = True
                    else:
                        print(
                            "Indigo version in {0} remains {1}...".format(
                                csproj_file, new_version
                            )
                        )
                    break
    if version_changed:
        tree.write(csproj_file)


def update_setup_py_version(setup_py_file, new_version_tuple):
    version_prefix, version_suffix, commits, _ = new_version_tuple
    new_version = version_prefix
    if version_suffix:
        if version_suffix == "alpha":
            version_suffix = "a"
        elif version_suffix == "beta":
            version_suffix = "b"
        new_version += "{}".format(version_suffix)
    if commits:
        dot_symbol = "." if not version_suffix else ""
        new_version += "{}{}".format(dot_symbol, commits)

    with open(setup_py_file) as f:
        data = f.read()
    vr = re.compile('version="(.+)"')
    m = vr.search(data)
    old_version = m.group(1)
    if old_version != new_version:
        data = vr.subn('version="{}"'.format(new_version), data)[0]
        print(
            "Updating Indigo version from {0} to {1} in {2}...".format(
                old_version, new_version, setup_py_file
            )
        )
        with open(setup_py_file, "w") as f:
            f.write(data)
    else:
        print("Indigo version in {0} remains {1}...".format(setup_py_file, new_version))


def update_toml_version(file_: str, version: str) -> None:
    config = configparser.ConfigParser()
    config.read(file_)
    # explicit quotes are required
    config["tool.poetry"]["version"] = f"'{version}'"
    with open(file_, "w") as toml_file:
        config.write(toml_file)


def update_init_version(file_: str, version: str) -> None:
    with open(file_, "w") as init_py:
        init_py.write(f"__version__ = '{version}'")


def update_bingo_elastic_py_version(
    bingo_elastic_path: str, new_version_tuple: str
) -> None:
    version_prefix, version_suffix, commits, _ = new_version_tuple
    files_ = {
        "pyproject.toml": update_toml_version,
        "bingo_elastic/__init__.py": update_init_version,
    }
    version = version_prefix
    for file_, updater in files_.items():
        updater(os.path.join(bingo_elastic_path, file_), version)


def main():
    sys.path.append(os.path.join(INDIGO_PATH, "api"))
    new_version = __import__("get_indigo_version").get_indigo_version_tuple_from_git()

    # update_pom_version(os.path.join(INDIGO_PATH, 'api', 'java', 'pom.xml'), new_version)
    # update_pom_version(os.path.join(INDIGO_PATH, 'api', 'plugins', 'bingo', 'java', 'pom.xml'), new_version)
    # update_pom_version(os.path.join(INDIGO_PATH, 'api', 'plugins', 'bingo-elastic', 'java', 'pom.xml'), new_version)
    # update_pom_version(os.path.join(INDIGO_PATH, 'api', 'plugins', 'inchi', 'java', 'pom.xml'), new_version)
    # update_pom_version(os.path.join(INDIGO_PATH, 'api', 'plugins', 'renderer', 'java', 'pom.xml'), new_version)
    # update_csproj_version(os.path.join(INDIGO_PATH, 'api', 'dotnet', 'Indigo.Net.csproj'), new_version)
    # update_setup_py_version(os.path.join(INDIGO_PATH, 'api', 'python', 'setup.py'), new_version)
    update_bingo_elastic_py_version(
        os.path.join(INDIGO_PATH, "api", "plugins", "bingo-elastic", "python"),
        new_version,
    )


if __name__ == "__main__":
    main()
