import os

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy, export_conandata_patches, get, load, replace_in_file, save, rename

required_conan_version = ">=1.53.0"


class CgltfConan(ConanFile):
    name = "cgltf"
    description = "Single-file glTF 2.0 loader and writer written in C99."
    license = "MIT"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://github.com/jkuhlmann/cgltf"
    topics = ("gltf", "header-only")

    package_type = "header-library"
    settings = "os", "arch", "compiler", "build_type"
    no_copy_source = True

    def layout(self):
        cmake_layout(self, src_folder="src")

    def package_id(self):
        self.info.clear()

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def package(self):
        copy(self, "LICENSE", self.source_folder, os.path.join(self.package_folder, "licenses"))
        for header in ["cgltf.h", "cgltf_write.h"]:
            copy(self, header, self.source_folder, os.path.join(self.package_folder, "include"))


    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "cgltf")
        self.cpp_info.set_property("cmake_target_name", "cgltf::cgltf")

        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
