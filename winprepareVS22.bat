ECHO Running cmake
cmake . -G "Visual Studio 17 2022" -Ax64 -Bbuild -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="cmake/conan_provider.cmake"
ECHO CMake has finished
pause
