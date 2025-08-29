{
  inputs = {
    # TODO: change to unstable, when node18 is dropped
    #nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    nixpkgs.url = "github:NixOS/nixpkgs?rev=e6031a08f659a178d0e4dcb9f3c8d065a0e6da4d";
    nixpkgs-unstable.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };
  outputs =
    inputs:
    inputs.flake-parts.lib.mkFlake { inherit inputs; } (
      { inputs, ... }:
      {
        systems = [
          "x86_64-linux"
          "aarch64-linux"
          "aarch64-darwin"
        ];

        perSystem =
          {
            pkgs,
            lib,
            self',
            inputs',
            ...
          }:
          let

            # TODO: remove when nixpkgs is updated
            webrtc-audio-processing = pkgs.webrtc-audio-processing_1.overrideAttrs (prevAttrs: {
              inherit (inputs'.nixpkgs-unstable.legacyPackages.webrtc-audio-processing) version src;

              mesonFlags =
                lib.lists.optional (!pkgs.stdenv.hostPlatform.isAarch64) "-Dneon=disabled"
                ++ lib.lists.optional (pkgs.stdenv.hostPlatform.isi686) "-Dinline-sse=false";
            });
          in
          {
            packages = {
              glad = pkgs.callPackage ./nix/glad.nix { };
              etc2comp = pkgs.callPackage ./nix/etc2comp.nix { };

              cgltf = pkgs.callPackage ./nix/cgltf.nix { };

              artery-font-format = pkgs.callPackage ./nix/artery-font-format.nix { };

              polyvox = pkgs.callPackage ./nix/polyvox.nix { };

              gif_creator = pkgs.callPackage ./nix/gif_creator.nix { };

              scribe = pkgs.callPackage ./nix/scribe.nix { };

              overte = pkgs.callPackage ./nix/overte.nix {
                inherit (self'.packages)
                  glad
                  scribe
                  gif_creator
                  polyvox
                  artery-font-format
                  cgltf
                  etc2comp
                  ;
                inherit webrtc-audio-processing;
                # FIXME: this doesn't work, bcs node18 uses icu75, which stops linking with overte
                libv8 = pkgs.nodejs_18.libv8;
              };
            };
            devShells.default = pkgs.mkShell {
              packages = [
                pkgs.conan
                pkgs.ninja
              ];
              inputsFrom = [ self'.packages.overte ];
              nativeBuildInputs = builtins.attrValues {
                inherit (pkgs)
                  cmake
                  pkg-config
                  python3
                  ;
              };
              buildInputs =
                builtins.attrValues {
                  inherit (pkgs)
                    openssl
                    libGL
                    glm
                    nlohmann_json
                    tbb_2022_0
                    nodejs_18
                    nvidia-texture-tools
                    openexr
                    draco
                    bullet
                    discord-rpc
                    openvr
                    openxr-loader
                    SDL2
                    libopus
                    ;
                  inherit (pkgs.libsForQt5)
                    qtbase
                    qtmultimedia
                    qtdeclarative
                    qtwebsockets
                    qtsvg
                    quazip
                    ;
                  inherit (pkgs.libsForQt5.qt5)
                    qtwebchannel
                    qtwebengine
                    qtxmlpatterns
                    qtquickcontrols
                    qtquickcontrols2
                    ;
                  inherit (self'.packages)
                    glad
                    etc2comp
                    cgltf
                    polyvox
                    gif_creator
                    artery-font-format
                    #libnode
                    ;
                  inherit webrtc-audio-processing;
                }
                ++ [
                  pkgs.nodejs_18.libv8
                ];
              NVTT_DIR = "${pkgs.nvidia-texture-tools}";
              GLSLANG_DIR = "${pkgs.glslang}/bin";
              SCRIBE_DIR = "${self'.packages.scribe}/tools";
              SPIRV_CROSS_DIR = "${pkgs.spirv-cross}/bin";
              SPIRV_TOOLS_DIR = "${pkgs.spirv-tools}/bin";
              # Enable function alignment to fix crashes in relation to V8 scripting engine
              CXXFLAGS = "-falign-functions";
            };
          };
      }
    );
}
