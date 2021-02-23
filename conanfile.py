import os
import platform

from conans import ConanFile, CMake


class IndigoConan(ConanFile):
    name = "indigo"
    version = "1.4.2"
    license = "Apache 2.0"
    url = "https://github.com/epam/indigo"
    description = "Universal cheminformatics toolkit"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    generators = "cmake"
    requires = ["zlib/1.2.11", "tinyxml/2.6.2", "rapidjson/cci.20200410"]
    default_options = {
        "shared": True
    }
    exports_sources = [".git/*", "api/*", "cmake/*", "core/*", "CMakeLists.txt", "conanfile.txt"]

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        if platform.system() == "Linux":
            prefix = 'lib'
            dynamic_suffix = '.so'
            static_suffix = '.a'
        elif platform.system() == 'Windows':
            prefix = ''
            dynamic_suffix = '.dll'
            static_suffix = '.lib'
        elif platform.system() == 'Darwin':
            prefix = 'lib'
            dynamic_suffix = '.dylib'
            static_suffix = '.a'
        else:
            raise ValueError(f'Unknown operating system: {platform.system()}')
        self.copy("indigo.h", dst="include", src="api/cpp/indigo")
        self.copy("indigo-renderer.h", dst="include", src="api/cpp/indigo-renderer")
        self.copy("indigo-inchi.h", dst="include", src="api/cpp/indigo-inchi")
        self.copy("bingo-nosql.h", dst="include", src="api/cpp/bingo-nosql")
        self.copy(f"*{prefix}indigo{dynamic_suffix}", dst="lib", keep_path=False)
        self.copy(f"*{prefix}indigo-renderer{dynamic_suffix}", dst="lib", keep_path=False)
        self.copy(f"*{prefix}indigo-inchi{dynamic_suffix}", dst="lib", keep_path=False)
        self.copy(f"*{prefix}bingo-nosql{dynamic_suffix}", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["indigo"]
