import os
from conan import ConanFile
from conan.tools.files import get, collect_libs
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout

class NTTConan(ConanFile):
    name = "nvidia-texture-tools"
    license = "MIT"
    url = "https://github.com/JulianGro/nvidia-texture-tools"
    description = "Texture processing tools with support for Direct3D 10 and 11 formats."
    settings = "os", "compiler", "build_type", "arch"

    def layout(self):
        cmake_layout(self)

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_TESTS"] = "OFF"
        tc.variables["BUILD_TOOLS"] = "OFF"
        tc.variables["USE_CUDA"] = "OFF"
        if self.settings.os == "Linux":
            tc.variables["CMAKE_CXX_FLAGS"] = "-march=msse3 -fPIC"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = collect_libs(self)
