{ stdenv, fetchFromGitHub }:
stdenv.mkDerivation rec {
  pname = "artery-font-format";
  version = "1.1";
  src = fetchFromGitHub {
    owner = "Chlumsky";
    repo = "artery-font-format";
    tag = "v${version}";
    hash = "sha256-JozvSQJt/62083sGX0jwMC2xmmpYrGvwd9MIfHtnqCs=";
  };

  installPhase = ''
    mkdir -p $out/include/
    cp -r artery-font $out/include/
  '';
}
