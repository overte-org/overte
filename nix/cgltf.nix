{
  stdenv,
  fetchFromGitHub,
}:
stdenv.mkDerivation rec {
  pname = "cgltf";
  version = "1.15";
  src = fetchFromGitHub {
    owner = "jkuhlmann";
    repo = "cgltf";
    tag = "v${version}";
    hash = "sha256-e+sVqcdOuLhsDHVntGzAwxtPxookrn706yEWDzNVvgI=";
  };
  outputs = [
    "out"
    "dev"
  ];
  installPhase = ''
    mkdir -p $dev/include $out
    cp cgltf* $dev/include/
  '';
}
