{
  stdenv,
  fetchFromBitbucket,
  cmake,
}:
stdenv.mkDerivation {
  pname = "polyvox";
  version = "0.2.1";
  src = fetchFromBitbucket {
    owner = "volumesoffun";
    repo = "polyvox";
    rev = "95f0aa22c12dde4b6e145f6b057a84cee006a5b0";
    hash = "sha256-Wc4b9RLOOLp62pHxGn6tplLlbJcPMq2Xf/2oUEPm/d4=";
  };
  nativeBuildInputs = [ cmake ];
  outputs = [
    "out"
  ];
}
