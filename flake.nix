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
            devShells.default = pkgs.mkShell {
              packages = [
                pkgs.conan
                pkgs.ninja
                pkgs.gdb
              ];
              inputsFrom = [ self'.packages.overte ];

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
