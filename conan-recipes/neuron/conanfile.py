from conan import ConanFile
from conan.tools.files import get, copy, collect_libs
import os


class NeuronConan(ConanFile):
    name = "neuron"
    version = "12.2"
    author = "Edgar (Edgar@AnotherFoxGuy.com)"
    settings = "os", "arch"

    def source(self):
        get(
            self,
            url="https://build-deps.overte.org/dependencies/neuron_datareader_b.12.2.zip",
            sha256="17a0e42f39aa38348c2c74bec2a011b75a512a0a18d9beb875ce62d4db005b23",
        )

    def package(self):
        if self.settings.os == "Windows":
            copy(
                self,
                "*.h",
                os.path.join(self.source_folder, "NeuronDataReader_Windows", "include"),
                os.path.join(self.package_folder, "include"),
            )
            copy(
                self,
                "*.lib",
                os.path.join(self.source_folder, "NeuronDataReader_Windows", "lib", "x64"),
                os.path.join(self.package_folder, "lib"),
                keep_path=False
            )
            copy(
                self,
                "*.dll",
                os.path.join(self.source_folder, "NeuronDataReader_Windows", "lib", "x64"),
                os.path.join(self.package_folder, "bin"),
                keep_path=False
            )
        elif self.settings.os == "Macos":
            copy(
                self,
                "*.h",
                os.path.join(self.source_folder, "NeuronDataReader_Mac", "include"),
                os.path.join(self.package_folder, "include"),
            )
            copy(
                self,
                "*.dylib",
                os.path.join(self.source_folder, "dylib"),
                os.path.join(self.package_folder, "lib"),
                keep_path=False
            )

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "Neuron")
        self.cpp_info.set_property("cmake_target_name", "Neuron::Datareader")
        self.cpp_info.libs = collect_libs(self)
