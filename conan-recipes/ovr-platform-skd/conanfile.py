from conan import ConanFile
from conan.tools.files import get, copy, collect_libs
import os


class OVRPlatformConan(ConanFile):
    name = "ovr-platform-skd"
    version = "1.10.0"
    author = "Edgar (Edgar@AnotherFoxGuy.com)"
    settings = "os", "arch"

    def build(self):
        get(
            self,
            url="https://build-deps.overte.org/dependencies/OVRPlatformSDK_v1.10.0.zip",
            sha256="4d0ecc491e4ddfc88056b674deef5a0a9a023d2f03b89e5ec6c1415863d200b2",
        )

    def package(self):
        copy(
            self,
            "*.h",
            os.path.join(self.source_folder, "Include"),
            os.path.join(self.package_folder, "include"),
        )
        copy(
            self,
            "LibOVRPlatform64_1.lib",
            os.path.join(self.source_folder, "Windows"),
            os.path.join(self.package_folder, "lib"),
            keep_path=False,
        )

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "OVRPlatform")
        self.cpp_info.set_property("cmake_target_name", "OVR::Platform")
        self.cpp_info.libs = collect_libs(self)
