{
  stdenv,
  fetchFromGitHub,
  cmake,
}:

stdenv.mkDerivation rec {
  version = "0.9.9.5";
  pname = "glm";

  src = fetchFromGitHub {
    owner = "g-truc";
    repo = pname;
    rev = version;
    sha256 = "sha256-1XKm1me72+pGJXfwbShu4TJsJZduLTqpdLLQZUPP314=";
  };

  outputs = [
    "out"
    "doc"
  ];

  nativeBuildInputs = [ cmake ];

  cmakeFlags = [
    "-DBUILD_SHARED_LIBS=OFF"
    "-DBUILD_STATIC_LIBS=OFF"
    "-DGLM_TEST_ENABLE=${if doCheck then "ON" else "OFF"}"
  ];

  doCheck = false;

  installPhase = ''
    runHook preInstall

    # Install header-only library
    mkdir -p $out/include
    cp -rv ../glm $out/include
    rm $out/include/glm/CMakeLists.txt
    rm $out/include/glm/detail/*.cpp

    # Install CMake files
    mkdir -p $out/lib
    cp -rv ../cmake $out/lib
    substituteInPlace $out/lib/cmake/glm/glmConfig.cmake \
        --replace 'GLM_INCLUDE_DIRS ''${_IMPORT_PREFIX}' "GLM_INCLUDE_DIRS $out/include"

    # Install pkg-config file
    mkdir -p $out/lib/pkgconfig
    substituteAll ${./glm.pc.in} $out/lib/pkgconfig/glm.pc

    # Install docs
    mkdir -p $doc/share/doc/glm
    cp -rv ../doc/api $doc/share/doc/glm/html
    cp -v ../doc/manual.pdf $doc/share/doc/glm

    runHook postInstall
  '';
}
