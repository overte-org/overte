from conan import ConanFile
from conan.tools.system import package_manager
from conan.tools.gnu import PkgConfig


class SysConfigOpenSSLConan(ConanFile):
    name = "openssl"
    version = "system"
    description = "cross-platform virtual conan package for the OpenSSL support"
    package_type = "shared-library"
    settings = "os", "arch", "compiler", "build_type"

    def layout(self):
        pass

    def package_id(self):
        self.info.clear()

    def system_requirements(self):
        apt = package_manager.Apt(self)
        apt.install(["libssl-dev"], check=True)

        pacman = package_manager.PacMan(self)
        pacman.install(["openssl"], check=True)

    def package_info(self):
        self.cpp_info.filenames["cmake_find_package"] = "openssl_system"
        self.cpp_info.filenames["cmake_find_package_multi"] = "openssl_system"

        self.cpp_info.set_property("cmake_file_name", "openssl_system")

        self.cpp_info.bindirs = []
        self.cpp_info.includedirs = []
        self.cpp_info.libdirs = []

        if self.settings.os == "Macos":
            self.cpp_info.frameworks.append("libssl")
            self.cpp_info.frameworks.append("libcrypto")
        elif self.settings.os == "Windows":
            self.cpp_info.system_libs = ["libssl", "libcrypto"]
        elif self.settings.os in ["Linux", "FreeBSD"]:
            pkg_config = PkgConfig(self, 'openssl')
            pkg_config.fill_cpp_info(self.cpp_info, is_system=self.settings.os != "FreeBSD")

