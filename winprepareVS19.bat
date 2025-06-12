ECHO Running cmake
cmake . -G"Visual Studio 16 2019" -Ax64 -Bbuild -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="cmake/conan_provider.cmake"
ECHO CMake has finished
pause