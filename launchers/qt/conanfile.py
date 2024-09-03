import os
from conan import ConanFile
from conan.tools.files import copy

class OVLauncher(ConanFile):
    name = "OVLauncher"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    default_options = {
        "*:shared": False,
        "qt*:gui": True,
        "qt*:qtquickcontrols": True,
        "qt*:qtquickcontrols2": True,
        "qt*:qtwebengine": False,
        "qt*:with_mysql": False,
        "qt*:with_odbc": False,
        "qt*:with_pq": False,
        "qt*:with_sqlite3": False,
    }

    def requirements(self):
        self.requires("qt/5.15.10")

    def generate(self):
        # Hack to get the static plugins to link
        for dir in self.dependencies["qt"].cpp_info.bindirs:
            copy(self, "*plugin.lib", dir, os.path.join(self.build_folder, "qtplugins"), False)

