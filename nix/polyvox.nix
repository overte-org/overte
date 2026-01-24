{
  stdenv,
  fetchFromGitHub,
  cmake,
}:
stdenv.mkDerivation rec {
  pname = "polyvox";
  version = "2025.09.1";
  src = fetchFromGitHub {
    owner = "overte-org";
    repo = "polyvox";
    tag = "v${version}";
    hash = "sha256-nUEc6p9WHbGsPAZUM0lFlogmiRoWzKSkdVChQKzffqI=";
  };
  cmakeFlags = [ "-DENABLE_EXAMPLES=OFF" ];
  nativeBuildInputs = [ cmake ];
  outputs = [
    "out"
  ];
}
