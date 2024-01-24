from conan import ConanFile
from conan.tools.files import get, copy, collect_libs
import os


class SteamWorksConan(ConanFile):
    name = "steamworks"
    version = "158a"
    author = "Edgar (Edgar@AnotherFoxGuy.com)"
    settings = "os", "arch"

    def source(self):
        get(
            self,
            url="https://build-deps.overte.org/dependencies/steamworks_sdk_158a.zip",
            sha256="69679da69158ac3f76e3ad955568e0d70925a092dd9407b88b0ab53160ee8991",
            strip_root=True,
        )

    def package(self):
        copy(
            self,
            "*.h",
            os.path.join(self.source_folder, "public"),
            os.path.join(self.package_folder, "include"),
        )
        if self.settings.os == "Windows":
            copy(
                self,
                "*.lib",
                os.path.join(self.source_folder, "redistributable_bin", "win64"),
                os.path.join(self.package_folder, "lib"),
                keep_path=False,
            )
            copy(
                self,
                "*.dll",
                os.path.join(self.source_folder, "redistributable_bin", "win64"),
                os.path.join(self.package_folder, "bin"),
                keep_path=False,
            )
        elif self.settings.os == "Macos":
            copy(
                self,
                "*.dylib",
                os.path.join(self.source_folder, "redistributable_bin", "osx"),
                os.path.join(self.package_folder, "lib"),
                keep_path=False,
            )
        else:
            copy(
                self,
                "*.so",
                os.path.join(self.source_folder, "redistributable_bin", "linux64"),
                os.path.join(self.package_folder, "lib"),
                keep_path=False,
            )

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "Steamworks")
        self.cpp_info.set_property("cmake_target_name", "Steam::Works")
        self.cpp_info.libs = collect_libs(self)
