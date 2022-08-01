import os
import platform
from ctypes import CDLL, RTLD_GLOBAL, c_void_p, sizeof


class Lib:
    @staticmethod
    def _system_name() -> str:
        return platform.system().lower().replace("msys_nt", "windows")

    @staticmethod
    def _machine_name() -> str:
        result = (
            platform.machine()
            .lower()
            .replace("amd64", "x86_64")
            .replace("arm64", "aarch64")
        )
        if result == "x86_64":
            if sizeof(c_void_p) == 4:
                result = "i386"
        return result

    @staticmethod
    def _library_prefix():
        system_name = Lib._system_name()
        if system_name in ("darwin", "linux"):
            library_prefix = "lib"
        elif system_name == "windows":
            library_prefix = ""
        else:
            raise RuntimeError("Unsupported OS")
        return library_prefix

    @staticmethod
    def _library_suffix():
        system_name = Lib._system_name()
        if system_name == "linux":
            library_suffix = "so"
        elif system_name == "darwin":
            library_suffix = "dylib"
        elif system_name == "windows":
            library_suffix = "dll"
        else:
            raise RuntimeError("Unsupported OS")
        return library_suffix

    @staticmethod
    def _library_path(name: str) -> str:
        library_base_path = os.path.join(
            os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "lib"
        )
        libraries_directory = "{}-{}".format(
            Lib._system_name(), Lib._machine_name()
        )
        library_name = "{}{}.{}".format(
            Lib._library_prefix(), name, Lib._library_suffix()
        )
        library_path = os.path.join(
            library_base_path, libraries_directory, library_name
        )
        if not os.path.exists(library_path):
            raise RuntimeError(
                "Could not find native libraries for target OS in "
                f"search directories: {library_path}"
            )
        return library_path

    @staticmethod
    def load(name: str) -> CDLL:
        return CDLL(Lib._library_path(name), mode=RTLD_GLOBAL)
