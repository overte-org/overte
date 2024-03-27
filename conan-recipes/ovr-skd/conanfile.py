from conan import ConanFile
from conan.tools.files import get, copy, collect_libs
from conan.tools.cmake import CMakeToolchain, CMake, CMakeDeps
import os


class OVRConan(ConanFile):
    name = "ovr-skd"
    version = "1.35.0"
    author = "Edgar (Edgar@AnotherFoxGuy.com)"
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "CMakeLists.txt"

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        strip_root = False
        if self.settings.os == "Windows":
            url = "https://build-deps.overte.org/dependencies/ovr_sdk_win_1.35.0.zip"
            sha256 = "2805619518a0a083f3eca0358ab7f4114d7d94b4abb2b65ced7e95f287df28a2"
        elif self.settings.os == "Macos":
            url = "https://build-deps.overte.org/dependencies/ovr_sdk_macos_0.5.0.1.tar.gz"
            sha256 = "58636983f970467afd18594899c22e70aae923023cd282b913c7c76ae46a8a12"
        else:
            url = "https://github.com/jherico/OculusSDK/archive/0d6f0cf110ea7566fc6d64b8d4fe6bb881d9cff5.zip"
            sha256 = "971e9f6ac8469913bd20445ba03a79e6654eaf71701823aa9fb5cec7c8e51ea6"
            strip_root = True
        get(self, url=url, sha256=sha256, strip_root=strip_root)
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "OVR")
        self.cpp_info.set_property("cmake_target_name", "OVR::SDK")
        self.cpp_info.libs = collect_libs(self)
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs.append("rt", "udev")
