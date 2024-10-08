from conan import ConanFile
from conan.tools.files import get, copy, collect_libs
import os


class GifCreatorConan(ConanFile):
    name = "gifcreator"
    version = "2016.11"
    author = "Edgar (Edgar@AnotherFoxGuy.com)"
    settings = "os", "arch"
    no_copy_source = True

    def source(self):
        get(
            self,
            url="https://build-deps.overte.org/dependencies/GifCreator.zip",
            sha256="26d20e3a889d967138e4fea3055fff6fb46930cbd0ed016fb9ab8bebd772018f",
            strip_root=True,
        )

    def package(self):
        copy(
            self,
            "*.h",
            self.source_folder,
            os.path.join(self.package_folder, "include"),
        )

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "GifCreator")
        self.cpp_info.set_property("cmake_target_name", "GifCreator::GifCreator")
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
