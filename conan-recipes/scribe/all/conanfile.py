import os
from conan import ConanFile
from conan.tools.files import get, collect_libs
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout

class ScribeConan(ConanFile):
    name = "scribe"
    license = "Apache License 2.0"
    url = "https://github.com/overte-org/scribe"
    description = "Scribe is a language for generating glsl files from templates"
    settings = "os", "compiler", "build_type", "arch"

    def layout(self):
        cmake_layout(self)

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.includedirs = []
        bin_dir = os.path.join(self.package_folder, "tools")
        self.output.info('Appending PATH environment variable: {}'.format(bin_dir))
        self.env_info.PATH.append(bin_dir)
