import os
from conan import ConanFile
from conan.tools.files import get, collect_libs
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout

class PolyvoxConan(ConanFile):
    name = "polyvox"
    license = "MIT"
    url = "https://git.anotherfoxguy.com/AnotherFoxGuy/polyvox/"
    description = "The voxel management and manipulation library"
    settings = "os", "compiler", "build_type", "arch"

    def layout(self):
        cmake_layout(self)

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["ENABLE_EXAMPLES"] = "OFF"
        tc.variables["ENABLE_TESTS"] = "OFF"
        tc.variables["BUILD_BINDINGS"] = "OFF"
        tc.variables["BUILD_DOCS"] = "OFF"
        tc.variables["BUILD_MANUAL"] = "OFF"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
