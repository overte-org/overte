{ stdenv, fetchzip }:
stdenv.mkDerivation {
  pname = "GifCreator";
  version = "2016.11";
  src = fetchzip {
    url = "https://build-deps.overte.org/dependencies/GifCreator.zip";
    hash = "sha256-WxYVy8T10ASsrvnHUdaoeh7befsFDlpR6ZsFtXTgFWA=";
  };

  installPhase = ''
    mkdir -p $out/include/GifCreator
    cp GifCreator.h $out/include/GifCreator/
  '';
}
