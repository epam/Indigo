import os
import platform
import re
import sys

REPO_ROOT = os.path.normpath(
    os.path.join(
        os.path.abspath(os.path.dirname(__file__)), "..", "..", "..", ".."
    )
)
system_name = None


def overridePlatform(platform):
    global system_name
    system_name = platform


def isIronPython():
    return sys.platform == "cli" or (
        "implementation" in dir(sys)
        and sys.implementation.name == "ironpython"
    )


def isJython():
    return os.name == "java"


def getIndigoVersion():
    version = ""
    cur_dir = os.path.split(__file__)[0]
    if not os.path.exists(
        os.path.join(cur_dir, "../../../indigo/api/indigo-version.cmake")
    ):
        return version
    for line in open(
        os.path.join(cur_dir, "../../../indigo/api/indigo-version.cmake")
    ):
        m = re.search('SET\(INDIGO_VERSION "(.*)"', line)
        if m:
            version = m.group(1)
    return version


def getCpuCount():
    if os.name == "java":
        from java.lang import Runtime

        runtime = Runtime.getRuntime()
        cpu_count = runtime.availableProcessors()
    else:
        import multiprocessing

        cpu_count = multiprocessing.cpu_count()
    return cpu_count


def getPlatform():
    global system_name
    if not system_name:
        if isJython():
            import java.lang.System

            osname = java.lang.System.getProperty("os.name")
            if osname.startswith("Windows"):
                system_name = "win"
            elif osname == "Mac OS X":
                system_name = "mac"
            elif osname == "Linux":
                system_name = "linux"
            else:
                raise EnvironmentError(
                    "Unsupported operating system %s" % osname
                )
        else:
            if os.name == "nt":
                if "GCC" in platform.python_compiler():
                    system_name = "mingw"
                else:
                    system_name = "win"
            elif os.name == "posix":
                if platform.mac_ver()[0]:
                    system_name = "mac"
                else:
                    system_name = "linux"
            else:
                raise EnvironmentError(
                    "Unsupported operating system %s" % os.name
                )
    return system_name


def get_indigo_java_version():
    import xml.etree.cElementTree as ElementTree

    pom_path = os.path.join(REPO_ROOT, "api", "java", "pom.xml")
    ElementTree.register_namespace("", "http://maven.apache.org/POM/4.0.0")
    tree = ElementTree.parse(pom_path)
    namespace = r"{http://maven.apache.org/POM/4.0.0}"
    indigo_version = None
    jna_version = None
    for l1_child in tree.getroot():
        if l1_child.tag == "{}properties".format(namespace):
            for l2_child in l1_child:
                if l2_child.tag == "{}revision".format(namespace):
                    indigo_version = l2_child.text
        if l1_child.tag == "{}dependencies".format(namespace):
            for l2_child in l1_child:
                if l2_child.tag == "{}dependency".format(namespace):
                    jna_found = False
                    for l3_child in l2_child:
                        if (
                            l3_child.tag == "{}artifactId".format(namespace)
                            and l3_child.text == "jna"
                        ):
                            jna_found = True
                            break
                    if jna_found:
                        for l3_child in l2_child:
                            if l3_child.tag == "{}version".format(namespace):
                                jna_version = l3_child.text
    if not indigo_version:
        raise ValueError(
            "Could not find Indigo version in {}".format(pom_path)
        )
    if not jna_version:
        raise ValueError("Could not find JNA version in {}".format(pom_path))
    return indigo_version, jna_version


def file_sha1(path):
    import hashlib

    sha1sum = hashlib.sha1()
    with open(path, "rb") as source:
        block = source.read(2 ** 16)
        while len(block) != 0:
            sha1sum.update(block)
            block = source.read(2 ** 16)
    return sha1sum.hexdigest()


def download_jna(jna_version, path):
    import urllib

    def check_jna_sha1():
        jna_sha1_url = "{}.sha1".format(jna_url)
        jna_ref_sha1 = urllib.urlopen(jna_sha1_url).read()
        jna_file_sha1 = file_sha1(output_path)
        if jna_ref_sha1 != jna_file_sha1:
            print(
                "Checked JNA at {}, sha1 {} is not equal to reference {}".format(
                    output_path, jna_file_sha1, jna_ref_sha1
                )
            )
            return False
        print(
            "Checked JNA at {}, sha1 {} verified".format(
                output_path, jna_file_sha1
            )
        )
        return True

    output_path = os.path.join(path, "jna-{}.jar".format(jna_version))
    jna_url = "https://search.maven.org/remotecontent?filepath=net/java/dev/jna/jna/{0}/jna-{0}.jar".format(
        jna_version
    )

    if os.path.exists(output_path) and check_jna_sha1():
        return

    try:
        import urllib

        urllib.urlretrieve(jna_url, output_path)
        if check_jna_sha1():
            print("Successfully downloaded JNA to {0}".format(output_path))
        else:
            raise RuntimeError(
                "Could not download and/or verify JNA from {}".format(jna_url)
            )
    except Exception as e:
        os.remove(output_path)
        raise e
