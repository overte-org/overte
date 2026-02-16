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

              overte-full = pkgs.callPackage ./nix/overte.nix {
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
                libv8 = pkgs.nodejs_22.libv8;
              };

              # TODO: update/remove when overte updates to more modern version
              draco = pkgs.callPackage ./nix/draco.nix { };

              glm = pkgs.callPackage ./nix/glm.nix { };

              default = self'.packages.overte-full;
            };
            devShells.default = pkgs.mkShell {
              packages = [
                pkgs.conan
                pkgs.ninja
                pkgs.gdb
              ];
              inputsFrom = [ self'.packages.overte-full ];

              buildInputs = [ pkgs.libsForQt5.full ];

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
            };
          };
      }
    );
}
