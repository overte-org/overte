<!--
Copyright 2013-2019 High Fidelity, Inc.
Copyright 2020-2021 Vircadia contributors
Copyright 2021-2022 Overte e.V.
SPDX-License-Identifier: Apache-2.0
-->

# General Build Information

*Last Updated on 21-10-2024*

## OS Specific Build Guides

* [Build Windows](BUILD_WIN.md) - complete instructions for Windows.
* [Build Linux](BUILD_LINUX.md) - additional instructions for Linux.
* [Build OSX](BUILD_OSX.md) - additional instructions for OS X.
* [Build Android](BUILD_ANDROID.md) - additional instructions for Android.

## Dependencies
- [git](https://git-scm.com/downloads): >= 1.6
- [CMake](https://cmake.org/download/):  3.9 (or greater up to 3.18.x)
- [Python](https://www.python.org/downloads/): 3.6 or higher
- [Node.JS](https://nodejs.org/en/): >= 12.13.1 LTS
    - Used to build the Screen Sharing executable.

## CMake External Project Dependencies

These dependencies need not be installed manually. They are automatically downloaded on the platforms where they are required.
- [Bullet Physics Engine](https://github.com/bulletphysics/bullet3/releases):  2.83
- [glm](https://glm.g-truc.net/0.9.8/index.html):  0.9.8
- [Oculus SDK](https://developer.oculus.com/downloads/):   1.11 (Windows) / 0.5 (Mac)
- [OpenVR](https://github.com/ValveSoftware/openvr):   1.11.11 (Windows, Linux)
- [Polyvox](http://www.volumesoffun.com/):   0.2.1
- [QuaZip](https://sourceforge.net/projects/quazip/files/quazip/):   0.7.3
- [SDL2](https://www.libsdl.org/download-2.0.php):   2.0.3
- [Intel Threading Building Blocks](https://www.threadingbuildingblocks.org/):   4.3
- [VHACD](https://github.com/virneo/v-hacd)
- [zlib](http://www.zlib.net/):   1.28 (Win32 only)
- [nvtt](https://github.com/hifi-archive/nvidia-texture-tools):   2.1.1 (customized)

The above dependencies will be downloaded, built, linked and included automatically by CMake where we require them. The CMakeLists files that handle grabbing each of the following external dependencies can be found in the [cmake/externals folder](cmake/externals). The resulting downloads, source files and binaries will be placed in the `build/ext` folder in each of the subfolders for each external project.

These are not placed in your normal build tree when doing an out of source build so that they do not need to be re-downloaded and re-compiled every time the CMake build folder is cleared. Should you want to force a re-download and re-compile of a specific external, you can simply remove that directory from the appropriate subfolder in `build/ext`. Should you want to force a re-download and re-compile of all externals, just remove the `build/ext` folder.

### CMake

Overte uses CMake to generate build files and project files for your platform.

### Qt

CMake will download Qt 5.15.2 using vcpkg.

To override this - i.e., use an installed Qt configuration - you need to set a QT_CMAKE_PREFIX_PATH environment variable pointing to your Qt **lib/cmake** folder.
This can either be entered directly into your shell session before you build or in your shell profile (e.g.: ~/.bash_profile, ~/.bashrc, ~/.zshrc - this depends on your shell and environment).  The path it needs to be set to will depend on where and how Qt5 was installed.

For example, under Linux:
```bash
export QT_CMAKE_PREFIX_PATH=/usr/local/Qt5.15.2/gcc_64/lib/cmake
export QT_CMAKE_PREFIX_PATH=/usr/local/qt/5.15.2/clang_64/lib/cmake/
export QT_CMAKE_PREFIX_PATH=/usr/local/Cellar/qt5/5.15.2/lib/cmake
export QT_CMAKE_PREFIX_PATH=/usr/local/opt/qt5/lib/cmake
```

For example, under Windows:

    set QT_CMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019_64\lib\cmake

For example, under OSX:

    export QT_CMAKE_PREFIX_PATH=/usr/local/Cellar/qt5/5.15.2/lib/cmake

Note: You only need the following components checked under Qt 5.15.2 (select the "Custom Installation" option):
"MSVC 2019 64-bit", "Qt WebEngine", and "Qt Script (Deprecated)".

Note: Installing the sources is optional but recommended if you have room for them (~3GB). You may also want the Qt debug
information files (~7GB).

Note: Installing Qt Creator is optional but recommended if you will be editing QML files.

### Conan

Overte uses conan to download and build dependencies.
Conan can be downloaded from here: https://conan.io/downloads

Building the dependencies can be lengthy and the resulting files will be stored in your home directory.
To move them to a different location, you can set the `CONAN_HOME` variable to any folder where you wish to install the dependencies.

Linux:

```bash
export CONAN_HOME=/path/to/directory
```

Windows:
```bash
set CONAN_HOME=/path/to/directory
```

Where `/path/to/directory` is the path to a directory where you wish the build files to get stored.

### Generating Build Files

#### Possible Environment Variables

```text
// The URL to post the dump to.
CMAKE_BACKTRACE_URL
// The identifying tag of the release.
CMAKE_BACKTRACE_TOKEN

// The release version, e.g., 2021.3.2.
RELEASE_NUMBER
// The build commit, e.g., use a Git hash for the most recent commit in the branch - fd6973b.
BUILD_NUMBER

// The type of release.
RELEASE_TYPE=PRODUCTION|PR|DEV

// The Interface will have a custom default home and startup location.
PRELOADED_STARTUP_LOCATION=Location/IP/URL
// The Interface will have a custom default script whitelist, comma separated, no spaces.
// This will also activate the whitelist on Interface's first run.
PRELOADED_SCRIPT_WHITELIST=ListOfEntries

// Code-signing environment variables must be set during runtime of CMake AND globally when the signing takes place.
HF_PFX_FILE=Path to certificate
HF_PFX_PASSPHRASE=Passphrase for certificate

// Determine the build type
PRODUCTION_BUILD=0|1
PR_BUILD=0|1
STABLE_BUILD=0|1

// Determine if to utilize testing or stable directory services URLs
USE_STABLE_GLOBAL_SERVICES=1
BUILD_GLOBAL_SERVICES=STABLE
```

#### Generate Files

```bash
conan install . -s build_type=Release -b missing -of build
cmake --preset conan-release
```

If CMake gives you the same error message repeatedly after the build fails, try removing `CMakeCache.txt`.

### Variables

Any variables that need to be set for CMake to find dependencies can be set as ENV variables in your shell profile, or passed directly to CMake with a `-D` flag appended to the `cmake ..` command.

For example, to pass the QT_CMAKE_PREFIX_PATH variable (if not using the vcpkg'ed version) during build file generation:

```bash
cmake --preset conan-release -DQT_CMAKE_PREFIX_PATH=/usr/local/qt/5.12.3/lib/cmake
```

### Finding Dependencies

The following applies for dependencies we do not grab via CMake ExternalProject (OpenSSL is an example), or for dependencies you have opted not to grab as a CMake ExternalProject (via -DUSE_LOCAL_$NAME=0). The list of dependencies we grab by default as external projects can be found in [the CMake External Project Dependencies section](#cmake-external-project-dependencies).

You can point our [CMake find modules](cmake/modules/) to the correct version of dependencies by setting one of the three following variables to the location of the correct version of the dependency.

In the examples below the variable $NAME would be replaced by the name of the dependency in uppercase, and $name would be replaced by the name of the dependency in lowercase (ex: OPENSSL_ROOT_DIR, openssl).

* $NAME_ROOT_DIR - pass this variable to Cmake with the -DNAME_ROOT_DIR= flag when running Cmake to generate build files
* $NAME_ROOT_DIR - set this variable in your ENV
* HIFI_LIB_DIR - set this variable in your ENV to your Overte lib folder, should contain a folder '$name'

## Optional Components

### Build Options

The following build options can be used when running CMake

* BUILD_CLIENT
* BUILD_SERVER
* BUILD_TESTS
* BUILD_TOOLS
* CLIENT_ONLY // Will package only the Interface
* SERVER_ONLY // Will package only the Server

### Optimization build options

* OVERTE_OPTIMIZE - This variable defaults to 1 if not set and enables compiler optimization flags on Linux and MacOS. Setting it to 0 will result in unoptimized build.
* OVERTE_CPU_ARCHITECTURE - This variable contains architecture specific compiler flags which are used if `OVERTE_OPTIMIZE` is true. If it is not set, it defaults to `-march=native -mtune=native`, which helps yield more performance for locally used build, but for packaging it needs to be set to different value for portability, for example `-msse3`. Setting `OVERTE_CPU_ARCHITECTURE` to empty string will use default compiler settings and yield
maximum compatibility.


### Developer Build Options

* USE_GLES
* DISABLE_UI

### Devices

You can support external input/output devices such as Leap Motion, MIDI, and more by adding each individual SDK in the visible building path. Refer to the readme file available in each device folder in [interface/external/](interface/external) for the detailed explanation of the requirements to use the device.

