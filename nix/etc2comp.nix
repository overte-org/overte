{
  stdenv,
  fetchFromGitHub,
  cmake,
}:
stdenv.mkDerivation {
  pname = "etc2comp";
  version = "unstable-2022-06-01";
  src = fetchFromGitHub {
    owner = "google";
    repo = "etc2comp";
    rev = "39422c1aa2f4889d636db5790af1d0be6ff3a226";
    hash = "sha256-mLr2Fsn9s9eWi71ljCll8c1hCk+dW6jCr7GG67ZTZ00=";
  };
  nativeBuildInputs = [ cmake ];
  outputs = [
    "out"
    "dev"
    "lib"
  ];
  installPhase = ''
    mkdir -p $lib/lib $dev/include $out/bin
    cp EtcLib/libEtcLib.a $lib/lib/
    cp EtcTool/EtcTool $out/bin/
    cp -r ../EtcLib $dev/include/
  '';
  meta.mainProgram = "EtcTool";
}
