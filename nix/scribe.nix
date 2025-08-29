{
  stdenv,
  fetchFromGitHub,
  cmake,
}:
stdenv.mkDerivation {
  pname = "scribe";
  version = "unstable";
  src = fetchFromGitHub {
    owner = "overte-org";
    repo = "scribe";
    rev = "6e10bcb440b1fee2da322c7405858532d0bf403e";
    hash = "sha256-fXhsoQBD0V8mS7roJtr2wt/ighW74U5LbasIHC3Q6JE=";
  };
  nativeBuildInputs = [ cmake ];
}
