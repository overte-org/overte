{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-25.11";
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
          {
            packages = {
              glad = pkgs.callPackage ./nix/glad.nix { };
              etc2comp = pkgs.callPackage ./nix/etc2comp.nix { };

              cgltf = pkgs.callPackage ./nix/cgltf.nix { };

              artery-font-format = pkgs.callPackage ./nix/artery-font-format.nix { };

              gif_creator = pkgs.callPackage ./nix/gif_creator.nix { };

              scribe = pkgs.callPackage ./nix/scribe.nix { };

              overte-full = pkgs.callPackage ./nix/overte.nix {
                inherit (self'.packages)
                  glad
                  scribe
                  gif_creator
                  artery-font-format
                  cgltf
                  etc2comp
                  draco
                  glm
                  ;
                libv8 = pkgs.nodejs_22.libv8;
              };

              # TODO: update/remove when overte updates to more modern version
              draco = pkgs.callPackage ./nix/draco.nix { };

              glm = pkgs.callPackage ./nix/glm.nix { };

              default = self'.packages.overte-full;
            };
            devShells.default = pkgs.mkShell {
              packages = [
                pkgs.ninja
                pkgs.gdb
                pkgs.clang-tools
              ];
              inputsFrom = [ self'.packages.overte-full ];

              buildInputs = [
                # taken from: https://github.com/NixOS/nixpkgs/blob/ac62194c3917d5f474c1a844b6fd6da2db95077d/pkgs/development/libraries/qt-5/5.15/default.nix#L353-L388
                (pkgs.qt5.env "overte-devenv" (
                  builtins.attrValues {
                    inherit (pkgs.libsForQt5)
                      quazip
                      ;
                    inherit (pkgs.qt5)
                      qt3d
                      qtcharts
                      qtconnectivity
                      qtdeclarative
                      qtdoc
                      qtgraphicaleffects
                      qtimageformats
                      qtmultimedia
                      qtquickcontrols
                      qtquickcontrols2
                      qtscript
                      qtsensors
                      qtserialport
                      qtsvg
                      qttools
                      qtwebchannel
                      qtwebengine
                      qtwebsockets
                      qtwebview
                      qtx11extras
                      qtxmlpatterns
                      ;
                  }
                ))
              ];

              CMAKE_GENERATOR = "Ninja";
              CMAKE_BUILD_TYPE = "Debug";
              CMAKE_EXPORT_COMPILE_COMMANDS = "ON";

              inherit (self'.packages.overte-full)
                NVTT_DIR
                CXXFLAGS
                GLSLANG_DIR
                SCRIBE_DIR
                SPIRV_CROSS_DIR
                SPIRV_TOOLS_DIR
                ;

              shellHook = ''
                # helper for configuring
                configure() {
                    cmake -DOVERTE_USE_SYSTEM_LIBS=ON -B build -S .
                }
              '';
            };
          };
      }
    );
}
