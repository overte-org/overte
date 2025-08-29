# copied from: https://github.com/NixOS/nixpkgs/blob/6a78816d7420ba5cb3093d4f0412d2bf0a57bfdd/pkgs/development/libraries/draco/default.nix
{
  stdenv,
  fetchFromGitHub,
  cmake,
}:
stdenv.mkDerivation rec {
  version = "1.3.5";
  pname = "draco";
  src = fetchFromGitHub {
    owner = "google";
    repo = "draco";
    rev = version;
    hash = "sha256-/p/hQbCGX6lunuO7ECjvrKHBi3jV6cSPdrU9MIxAVhk=";
  };
  patches = [ ./draco.diff ];
  enableParallelBuilding = true;
  nativeBuildInputs = [ cmake ];
  cmakeFlags = [
    # Fake these since we are building from a tarball
    "-Ddraco_git_hash=${version}"
    "-Ddraco_git_desc=${version}"
    "-DBUILD_UNITY_PLUGIN=1"
  ];
}
