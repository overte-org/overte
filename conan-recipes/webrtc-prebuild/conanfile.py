from conan import ConanFile
from conan.tools.files import get, copy
import os


class WebRTCConan(ConanFile):
    name = "webrtc"
    version = "2021.01.05"
    author = "Edgar (Edgar@AnotherFoxGuy.com)"
    settings = "os", "arch"

    def build(self):
        if self.settings.os == "Windows":
            url = "https://build-deps.overte.org/dependencies/vcpkg/webrtc-m84-20210105-windows.zip"
            sha256 = "0af3da6d7dec42a87d0f6b4917d9a4412233ee6b280110495429219e37fbdf47"
        elif self.settings.os == "Macos":
            url = "https://build-deps.overte.org/seth/webrtc-m78-osx.tar.gz"
            sha256 = "3ce69c3761ab41ad3a861caee3cb0a6140ebb3711035458226f99df75ad10836"
        else:
            url = "https://build-deps.overte.org/dependencies/vcpkg/webrtc-m84-gcc-linux.tar.xz"
            sha256 = "c1da57621c3c9fdc8f2c106f401931f3d818914e91f82006681ff1e50c5db815"
        get(self, url=url, sha256=sha256, strip_root=True)

    def package(self):
        copy(
            self,
            "*",
            os.path.join(self.source_folder, "include"),
            os.path.join(self.package_folder, "include"),
        )
        copy(
            self,
            "*",
            os.path.join(self.source_folder, "lib"),
            os.path.join(self.package_folder, "lib"),
            keep_path=False,
        )

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "WebRTC")
        self.cpp_info.set_property("cmake_target_name", "WebRTC::WebRTC")
        self.cpp_info.libs = ["webrtc"]
