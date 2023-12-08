import os
from conan import ConanFile
from conan.tools.files import get, collect_libs
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout

class libloConan(ConanFile):
    name = "liblo"
    license = "MIT"
    url = "https://github.com/radarsat1/liblo"
    description = "liblo is an implementation of the Open Sound Control protocol for POSIX systems"
    settings = "os", "compiler", "build_type", "arch"

    def layout(self):
        cmake_layout(self)

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["WITH_TOOLS"] = "OFF"
        tc.variables["WITH_TESTS"] = "OFF"
        tc.variables["WITH_EXAMPLES"] = "OFF"
        tc.variables["WITH_CPP_TESTS"] = "OFF"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure(build_script_folder="cmake")
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = collect_libs(self)