{
  # build tools
  lib,
  stdenv,
  cmake,
  pkg-config,
  autoPatchelfHook,

  # dependencies
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

  # tools for shader compilation
  scribe,
  glslang,
  spirv-tools,
  spirv-cross,
  python3,

  # build options
  buildClient ? true,
  buildServer ? true,
  buildTools ? false,
}:
stdenv.mkDerivation {
  pname = "overte";
  version = "unstable";
  src =
    let
      fs = lib.fileset;
    in
    fs.toSource {
      root = ./..;
      fileset = fs.intersection (fs.gitTracked ./..) (
        fs.unions [
          ./../interface
          ./../plugins
          ./../tools
          ./../domain-server
          ./../assignment-client
          ./../server-console
          ./../ice-server
          ./../cmake
          ./../libraries
          ./../scripts
          ./../CMakeLists.txt
          ./../LICENSE
        ]
      );
    };
  nativeBuildInputs = [
    cmake
    pkg-config
    python3
    libsForQt5.wrapQtAppsHook
    nodejs
    autoPatchelfHook
  ];

  # TODO: make dependencies minimal for !buildClient
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
        qtx11extras
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
  ]
  ++ lib.optional (!buildClient) "-DOVERTE_BUILD_CLIENT=OFF"
  ++ lib.optional (!buildServer) "-DOVERTE_BUILD_SERVER=OFF"
  ++ lib.optional (!buildTools && !buildServer) "-DOVERTE_BUILD_TOOLS=OFF";
  env = {
    NVTT_DIR = "${nvidia-texture-tools}";
    CXXFLAGS = "-falign-functions=32";
    GLSLANG_DIR = "${glslang}/bin";
    SCRIBE_DIR = "${scribe}/tools";
    SPIRV_CROSS_DIR = "${spirv-cross}/bin";
    SPIRV_TOOLS_DIR = "${spirv-tools}/bin";
  };

  dontWrapQtApps = true;

  # TODO: remove set QT_PLUGIN_PATH after qt6 update
  installPhase = ''
    runHook preInstall

    mkdir -p $out/{bin/,lib/}
    cp -r libraries/**/*.so $out/lib/
  ''
  + (lib.optionalString buildClient # sh
    ''
      I="$out/interface"
      mkdir -p $I
      cp interface/resources.rcc "$I"/
      cp -r interface/scripts "$I"/scripts
      cp -r interface/resources "$I"/resources
      cp -r interface/plugins "$I"/plugins


      cp interface/interface "$I"/
      ln -s "$I"/interface $out/bin/overte-client
      makeWrapper "$I"/interface $out/bin/overte-client \
        --set QT_PLUGIN_PATH ''' \
        "''${qtWrapperArgs[@]}"
    ''
  )
  + (lib.optionalString buildServer # sh
    ''
      D="$out/domain-server"
      mkdir -p $D
      cp -r domain-server/resources "$D"/resources

      cp domain-server/domain-server "$D"/domain-server
      makeWrapper "$D"/domain-server $out/bin/domain-server \
        "''${qtWrapperArgs[@]}"

      cp assignment-client/assignment-client "$D"/assignment-client
      makeWrapper "$D"/assignment-client $out/bin/assignment-client \
        "''${qtWrapperArgs[@]}"

      cp tools/oven/oven "$D"/oven
      makeWrapper "$D"/oven $out/bin/oven \
        "''${qtWrapperArgs[@]}"
    ''
  )
  # sh
  + ''
    for file in $(find $out -type f \( -perm /0111 -o -name \*.so\* \) ); do
        patchelf --set-interpreter "$(cat $NIX_CC/nix-support/dynamic-linker)" "$file" || true
        patchelf --remove-rpath $file || true
    done
    runHook postInstall
  '';

  meta = {
    mainProgram = if buildClient then "overte-client" else "domain-server";
  };
}
