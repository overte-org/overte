import os
from conan import ConanFile
from conan.tools.files import copy

class Overte(ConanFile):
    name = "Overte"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    default_options = {
        "qt*:shared": "True",
        "qt*:gui": "True",
        "qt*:qtdeclarative": "True",
        "qt*:qtlocation": "True",
        "qt*:qtmultimedia": "True",
        "qt*:qtquickcontrols2": "True",
        "qt*:qtscxml": "True",
        "qt*:qtsvg": "True",
        "qt*:qtwebchannel": "True",
        "qt*:qtwebengine": "True",
        "qt*:qtwebsockets": "True",
        "qt*:qtwebview": "True",
        "qt*:qtxmlpatterns": "True",
    }

    def requirements(self):
        self.requires("bullet3/3.25")
        self.requires("draco/1.5.6")
        self.requires("etc2comp/cci.20170424")
        self.requires("glad/0.1.36")
        self.requires("gli/cci.20210515")
        self.requires("glm/cci.20230113")
        self.requires("glslang/11.7.0")
        self.requires("liblo/0.30@overte/stable")
        self.requires("libnode/18.17.1@overte/stable")
        self.requires("nlohmann_json/3.11.2")
        self.requires("nvidia-texture-tools/2023.01@overte/stable")
        self.requires("openexr/3.1.9")
        self.requires("openssl/1.1.1w")
        self.requires("openvr/1.26.7@overte/stable")
        self.requires("opus/1.3.1")
        self.requires("polyvox/2016.11@overte/stable")
        #self.requires("quazip/1.4")
        self.requires("sdl/2.26.5")
        #self.requires("qt/5.15.10", force=True)
        self.requires("scribe/2019.02@overte/stable")
        #self.requires("shaderc/2021.1") # Broken
        self.requires("spirv-cross/cci.20211113")
        self.requires("spirv-tools/2021.4")
        self.requires("onetbb/2021.10.0")
        self.requires("v-hacd/4.1.0")
        self.requires("vulkan-memory-allocator/3.0.1")
        # webrtc
        self.requires("zlib/1.2.13")

    # def generate(self):
    #     for dep in self.dependencies.values():
    #         for f in dep.cpp_info.bindirs:
    #             self.cp_data(f)
    #         for f in dep.cpp_info.libdirs:
    #             self.cp_data(f)

    def cp_data(self, src):
        bindir = os.path.join(self.build_folder, "bin")
        copy(self, "*.dll", src, bindir, False)
        copy(self, "*.so*", src, bindir, False)
