{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    rust-overlay.url = "github:oxalica/rust-overlay";
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
            system,
            lib,
            self',
            inputs',
            ...
          }:
          {
            _module.args.pkgs = import inputs.nixpkgs {
              inherit system;
              overlays = [
                (import inputs.rust-overlay)
                (self: _super: {
                  rust-overte = self.rust-bin.fromRustupToolchainFile ./rust-toolchain.toml;
                })
              ];
            };
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
                pkgs.conan
                pkgs.ninja
                pkgs.gdb
                pkgs.nixd
                pkgs.nixfmt
                pkgs.clang-tools
                pkgs.llvmPackages_20.clang-unwrapped
                pkgs.rust-overte
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
