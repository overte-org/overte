<!--
Copyright 2013-2019 High Fidelity, Inc.
Copyright 2020-2021 Vircadia contributors
Copyright 2020-2025 Overte e.V.
SPDX-License-Identifier: Apache-2.0
-->

# Build macOS

*Last Updated on August 12, 2025*

Please read the [general build guide](BUILD.md) for information on dependencies required for all platforms.
This will include the necessary environment variables to customize your build. Only macOS specific instructions are found in this document.

## Prerequisites

### CMake, OpenSSL, NPM and Conan

[Homebrew](https://brew.sh/) is an excellent package manager for macOS. It makes the installation of some Overte dependencies very simple.

```bash
brew install cmake npm conan
```

**Note:** You can also download alternative CMake versions from [Github](https://github.com/Kitware/CMake/releases) if needed.

### Qt5

While Conan can build Qt from source, we use the Homebrew package instead:
```bash
brew install qt@5
```

### Prepare conan

The next step is setting up conan

First, create a conan profile
```bash
conan profile detect --force
```

Next, add the overte remote to conan
```bash
conan remote add overte https://artifactory.overte.org/artifactory/api/conan/overte -f
```

Add CMake 4.0 is currently too new for us, so we tell Conan to get and use an older version.
Add the following to the default Conan profile (which is usually found in `~/.conan2/profiles/default`):
```text
[tool_requires]
!cmake/*: cmake/[>=3 <4]
```

## Compiling

If you installed Qt5 as instructed above, we need to add its path to the environment as instructed by Homebrew:
```bash
export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"
```
If you only need Qt5 on your machine (for example if you only develop Overte), you can make this change permanent as well:
```bash
echo 'export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"' >> ~/.zshrc
```

Install the dependencies with conan
```bash
cd overte
conan install . -s build_type=Release -b missing -pr:b=default -of build
```

## Generate and Build

You can choose to use either Unix Makefiles or Xcode.

### make

Run CMake.

Prepare makefiles:
```bash
cmake --preset conan-release
```

Build:
```bash
cmake --build --preset conan-release
```
Keep in mind that the `interface` target is called `overte` on macOS.

To package the installation, you can simply run `cmake --build --preset conan-release --target package` afterwards.

### Xcode

You can ask CMake to generate Xcode project files instead of Unix Makefiles using the `-G Xcode` parameter after CMake.
You will need to select the Xcode installation in the terminal first if you have not done so already.

```bash
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
cmake --preset conan-release -G Xcode
```

After running CMake, you will have the Xcode project file necessary to build all of the components.
Open the `overte.xcodeproj` file, choose `ALL_BUILD` from the Product > Scheme menu (or target drop down), and click Run.

If the build completes successfully, you will have built targets for all components located in the `build/${target_name}/Debug` directories.

## FAQ

1. **Problem:** Running the scheme `interface.app` from Xcode causes a crash for Interface related to `libgl`.
    1. **Cause:** The target `gl` generates a binary called `libgl`. A macOS `libGL.framework` item gets loaded instead by Xcode.
    2. **Solution:** In the Xcode target settings for `libgl`, set the version to `1.0.0`.
2. **Problem:** CMake complains about Python 3 being missing.
    1. **Cause:** CMake might be out of date.
    2. **Solution:** Try updating your CMake binary with command `brew upgrade cmake`, or by downloading and running a newer CMake installer,
                     depending on how you originally installed CMake. Please keep in mind the recommended CMake versions noted above.
