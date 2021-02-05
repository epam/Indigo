import glob
import os
import shutil
import subprocess
import sys
from optparse import OptionParser
from typing import Set
from contextlib import contextmanager

parser = OptionParser(description="Indigo Python libraries build script")
parser.add_option("--suffix", "-s", help="archive suffix", default="")
parser.add_option(
    "--publish", help="Publish wheels", default=False, action="store_true"
)
(args, left_args) = parser.parse_args()


def make_zips(api_dir, dist_dir):
    # Find indigo version
    from get_indigo_version import getIndigoVersion

    version = getIndigoVersion()

    if not os.path.exists(dist_dir):
        os.mkdir(dist_dir)

    archive_name = "./indigo-python-%s-%s" % (version, args.suffix)

    dest = os.path.join(dist_dir, archive_name)
    if os.path.exists(dest):
        shutil.rmtree(dest)
    os.mkdir(dest)
    os.mkdir(os.path.join(dest, "indigo"))
    shutil.copy(
        os.path.join(api_dir, "python", "indigo.py"),
        os.path.join(dest, "indigo", "__init__.py"),
    )
    shutil.copy(
        os.path.join(api_dir, "plugins", "renderer", "python", "indigo_renderer.py"),
        dest,
    )
    shutil.copy(
        os.path.join(api_dir, "plugins", "renderer", "python", "indigo_renderer.py"),
        os.path.join(dest, "indigo", "renderer.py"),
    )
    shutil.copy(
        os.path.join(api_dir, "plugins", "inchi", "python", "indigo_inchi.py"), dest
    )
    shutil.copy(
        os.path.join(api_dir, "plugins", "inchi", "python", "indigo_inchi.py"),
        os.path.join(dest, "indigo", "inchi.py"),
    )
    shutil.copy(os.path.join(api_dir, "plugins", "bingo", "python", "bingo.py"), dest)
    shutil.copy(
        os.path.join(api_dir, "plugins", "bingo", "python", "bingo.py"),
        os.path.join(dest, "indigo", "bingo.py"),
    )
    shutil.copytree(
        os.path.join(api_dir, "libs", "shared"),
        os.path.join(dest, "lib"),
        ignore=shutil.ignore_patterns("*.lib"),
    )

    shutil.copy(os.path.join(api_dir, "LICENSE"), dest)
    os.chdir(dist_dir)
    if os.path.exists(archive_name + ".zip"):
        os.remove(archive_name + ".zip")
    shutil.make_archive(
        archive_name, "zip", os.path.dirname(archive_name), archive_name
    )
    shutil.rmtree(archive_name)
    full_archive_name = os.path.normpath(os.path.join(dist_dir, archive_name))
    print("Archive {}.zip created".format(full_archive_name))


def execute_setup_py(archs: Set[str]) -> bool:
    """Executes api/python/setup.py to every architecture from archs"""
    all_success = True
    for arch in archs:
        command = [
            sys.executable,
            "setup.py",
            "bdist_wheel",
            f"--plat-name={arch}",
        ]
        try:
            print(f"Running setup.py with --plat-name={arch}")
            subprocess.check_call(command)
        except subprocess.CalledProcessError:
            print("Error in running command:")
            print(" ".join(command))
            all_success = False
    return all_success


def get_archs(dist_dir: str) -> Set[str]:
    """Returns a set of architectures based on directory listing."""
    result = set()
    for file_ in os.listdir(dist_dir):
        if "zip" == file_.split(".")[-1]:
            if "mac" in file_:
                result.add("macosx_10_7_intel")
            elif "win" in file_:
                result.add("win_amd64")
            elif "linux" in file_:
                result.add("manylinux1_x86_64")

    return result


@contextmanager
def prepare_dirs(api_dir: str, dest: str):
    if os.path.exists(dest):
        shutil.rmtree(dest)
    os.makedirs(dest)
    os.makedirs(os.path.join(dest, "indigo"))

    shutil.copy(os.path.join(api_dir, "LICENSE"), dest)
    shutil.copy(
        os.path.join(api_dir, "python", "indigo.py"),
        os.path.join(dest, "indigo", "__init__.py"),
    )
    shutil.copy(
        os.path.join(api_dir, "plugins", "renderer", "python", "indigo_renderer.py"),
        os.path.join(dest, "indigo", "renderer.py"),
    )
    shutil.copy(
        os.path.join(api_dir, "plugins", "inchi", "python", "indigo_inchi.py"),
        os.path.join(dest, "indigo", "inchi.py"),
    )
    shutil.copy(
        os.path.join(api_dir, "plugins", "bingo", "python", "bingo.py"),
        os.path.join(dest, "indigo", "bingo.py"),
    )
    shutil.copy(os.path.join(api_dir, "python", "setup.py"), dest)
    shutil.copytree(
        os.path.join(api_dir, "libs", "shared"),
        os.path.join(dest, "indigo", "lib"),
        ignore=shutil.ignore_patterns("*.lib"),
    )
    cur_dir = os.path.abspath(os.curdir)
    os.chdir(dest)
    try:
        yield
    finally:
        os.chdir(cur_dir)
        shutil.rmtree(os.path.join(dest, "build"))
        shutil.rmtree(os.path.join(dest, "epam.indigo.egg-info"))
        shutil.rmtree(os.path.join(dest, "indigo"))
        os.remove(os.path.join(dest, "LICENSE"))
        os.remove(os.path.join(dest, "setup.py"))
        for file in glob.glob(os.path.join(dest, "dist", "*.whl")):
            shutil.move(file, os.path.join(cur_dir, os.path.basename(file)))
        shutil.rmtree(os.path.join(cur_dir, "epam.indigo"))


def make_wheels(api_dir: str, archs: Set[str], dest: str) -> bool:

    with prepare_dirs(api_dir, dest):
        is_success = execute_setup_py(archs)
        if args.publish and is_success:
            subprocess.check_call(
                [
                    "twine",
                    "upload",
                    "-u",
                    "__token__",
                    "-p",
                    os.environ["PYPI_TOKEN"],
                    "dist/*.whl",
                ]
            )
        return is_success


if __name__ == "__main__":
    api_dir = os.path.abspath(os.path.dirname(__file__))
    root = os.path.normpath(os.path.join(api_dir, ".."))
    dist_dir = os.path.join(root, "dist")

    architectures = get_archs(dist_dir)
    make_zips(api_dir, dist_dir)
    is_success = make_wheels(
        api_dir, architectures, os.path.join(dist_dir, "epam.indigo")
    )
    if not is_success:
        exit(1)
