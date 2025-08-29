{
  stdenv,
  cmake,
  pkg-config,
  autoPatchelfHook,

  glad,
  gif_creator,
  polyvox,
  artery-font-format,
  cgltf,
  etc2comp,
  openssl,
  libGL,
  glm,
  nlohmann_json,
  tbb_2022,
  nodejs,
  webrtc-audio-processing,
  nvidia-texture-tools,
  openexr,
  draco,
  bullet,
  discord-rpc,
  openvr,
  openxr-loader,
  SDL2,
  libopus,
  libsForQt5,
  libv8,

  scribe,
  glslang,
  spirv-tools,
  spirv-cross,
  python3,
}:
stdenv.mkDerivation {
  pname = "overte";
  version = "unstable";
  src = ./..;
  nativeBuildInputs = [
    cmake
    pkg-config
    python3
    libsForQt5.wrapQtAppsHook
    nodejs
    autoPatchelfHook
  ];
  buildInputs =
    builtins.attrValues {
      inherit (libsForQt5)
        qtbase
        qtmultimedia
        qtdeclarative
        qtwebsockets
        qtsvg
        quazip
        ;
      inherit (libsForQt5.qt5)
        qtwebchannel
        qtwebengine
        qtxmlpatterns
        qtquickcontrols2
        qtgraphicaleffects
        ;
    }
    ++ [
      glad
      etc2comp
      cgltf
      polyvox
      gif_creator
      artery-font-format
      openssl
      libGL
      glm
      nlohmann_json
      tbb_2022
      webrtc-audio-processing
      nvidia-texture-tools
      openexr
      draco
      bullet
      discord-rpc
      openvr
      openxr-loader
      SDL2
      libopus
      libv8
    ];
  cmakeFlags = [
    "-DOVERTE_USE_SYSTEM_LIBS=ON"
    "-DOVERTE_BUILD_TYPE=NIGHLTY"
  ];
  env = {
    NVTT_DIR = "${nvidia-texture-tools}";
    CXXFLAGS = "-falign-functions";
    GLSLANG_DIR = "${glslang}/bin";
    SCRIBE_DIR = "${scribe}/tools";
    SPIRV_CROSS_DIR = "${spirv-cross}/bin";
    SPIRV_TOOLS_DIR = "${spirv-tools}/bin";
  };

  installPhase = ''
    runHook preInstall

    I="$out/interface"
    mkdir -p $I $out/{bin/,lib/}
    cp interface/interface "$I"/
    cp interface/resources.rcc "$I"/
    cp -r interface/scripts "$I"/scripts
    cp -r libraries/**/*.so $out/lib/
    cp -r interface/plugins "$I"/plugins

    for file in $(find $out -type f \( -perm /0111 -o -name \*.so\* \) ); do
      patchelf --set-interpreter "$(cat $NIX_CC/nix-support/dynamic-linker)" "$file" || true
      patchelf --remove-rpath $file || true
    done

    makeWrapper "$I"/interface "$out/bin/overte" \
        --inherit-argv0

    runHook postInstall
  '';

  /*
    preFixup = ''
      patchelf --read-rpath $out/interface/plugins/libopenvr.so
      ldd $out/interface/plugins/*.so
    '';
  */

  meta = {
    mainProgram = "overte";
  };
}
