{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
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
                  draco
                  glm
                  ;
                libv8 = pkgs.nodejs.libv8;
              };

              # TODO: update/remove when overte updates to more modern version
              draco = pkgs.callPackage ./nix/draco.nix { };

              glm = pkgs.callPackage ./nix/glm.nix { };
            };
            devShells = {
              default = pkgs.mkShell {
                packages = [
                  pkgs.conan
                  pkgs.ninja
                  pkgs.gdb
                ];
                inputsFrom = [ self'.packages.overte ];

                buildInputs = [ pkgs.qt6.full ];

                inherit (self'.packages.overte)
                  NVTT_DIR
                  CXXFLAGS
                  GLSLANG_DIR
                  SCRIBE_DIR
                  SPIRV_CROSS_DIR
                  SPIRV_TOOLS_DIR
                  ;
              };
              fhs =
                (pkgs.buildFHSEnv {
                  name = "overte-env";
                  targetPkgs =
                    pkgs:
                    with pkgs;
                    [
                      pkg-config
                      cmake
                      gcc
                      ninja
                      conan
                      gdb

                      # aqt requirements
                      python3
                      bzip2

                      libGL
                      openssl

                      xorg.libX11
                      xorg.libXi
                      xorg.libXmu
                      xorg.libXext
                      xorg.libXfixes
                      xorg.libXcomposite
                      xorg.libXtst
                      xorg.libXrandr
                      xorg.libXdmcp
                      xorg.libXdamage
                      xorg.libXcursor
                      xorg.libxcb
                      xorg.libXrender
                      libxcb
                      libxau
                      libxkbcommon
                      xorg.xcbproto
                      xorg.xorgproto
                      xorg.xrandr

                      glib
                      expat
                      fontconfig
                      dbus
                      libgssglue
                      krb5
                      pulseaudio
                      nss
                      nspr
                      freetype
                      alsa-oss
                      alsa-lib

                      wayland
                      libffi
                      vulkan-loader
                    ]
                    # TODO: replace with qt6 and  when in main branch
                    ++ (with qt6Packages; [
                      qtbase
                      qtmultimedia
                      qtdeclarative
                      qtwebsockets
                      qtsvg
                      quazip
                      qtwebchannel
                      qtwebengine
                      qtpositioning
                      qt5compat
                    ]);

                  extraOutputsToInstall = [ "dev" ];
                  extraBuildCommands = ''
                    mkdir -p $out/usr/mkspecs
                    cp -rsHf ${pkgs.qt6Packages.qtbase}/mkspecs/* $out/usr/mkspecs/
                  '';

                  profile = ''
                    export PKG_CONFIG_PATH="/usr/share/pkgconfig:''${PKG_CONFIG_PATH}"
                    export X11_ROOT="/usr"
                    export BZip2_ROOT="/usr"
                    export NIXPKGS_CMAKE_PREFIX_PATH="/usr"
                  '';
                }).env;

            };
          };
      }
    );
}
