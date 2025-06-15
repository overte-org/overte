ECHO Running conan
conan install . -b missing -pr=tools/conan-profiles/vs-22-release -of build
conan install . -b missing -pr=tools/conan-profiles/vs-22-debug -of build
conan cache clean "*" -sbd
ECHO Running cmake
cmake --preset conan-default
ECHO CMake has finished
pause