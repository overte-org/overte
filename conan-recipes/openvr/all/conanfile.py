import os
from conan import ConanFile
from conan.tools.files import get, collect_libs, replace_in_file, rmdir, copy
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class OpenvrConan(ConanFile):
    name = "openvr"
    description = "API and runtime that allows access to VR hardware from applications have specific knowledge of the hardware they are targeting."
    topics = (
        "conan",
        "openvr",
        "vr",
    )
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://github.com/ValveSoftware/openvr"
    license = "BSD-3-Clause"
    settings = "os", "compiler", "build_type", "arch"
    implements = ["auto_shared_fpic"]
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("jsoncpp/1.9.5")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)
        # Unvendor jsoncpp (we rely on our CMake wrapper for jsoncpp injection)
        rmdir(self, os.path.join(self.source_folder, "src", "json"))
        replace_in_file(
            self,
            os.path.join(self.source_folder, "src", "CMakeLists.txt"),
            "jsoncpp.cpp",
            "",
        )
        replace_in_file(
            self,
            os.path.join(self.source_folder, "src", "CMakeLists.txt"),
            "target_link_libraries(${LIBNAME} ${EXTRA_LIBS} ${CMAKE_DL_LIBS})",
            """
            find_package(jsoncpp)
            target_link_libraries(${LIBNAME} JsonCpp::JsonCpp ${EXTRA_LIBS} ${CMAKE_DL_LIBS})
            """,
        )

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_UNIVERSAL"] = "OFF"
        tc.variables["BUILD_SHARED"] = self.options.shared
        tc.variables["USE_LIBCXX"] = "OFF"
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE", src=self.source_folder, dst="licenses")
        cmake = CMake(self)
        cmake.install()
        copy(
            self,
            pattern="*.dll",
            dst="bin",
            src=os.path.join(self.build_folder, "bin", "win64"),
            keep_path=False,
        )

    def package_info(self):
        if not self.options.shared:
            self.cpp_info.defines = ["OPENVR_BUILD_STATIC"]
        self.cpp_info.names["pkg_config"] = "openvr"
        self.cpp_info.libs = collect_libs(self)
        self.cpp_info.includedirs.append(os.path.join("include", "openvr"))

        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs.append("dl")
