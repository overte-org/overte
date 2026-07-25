<!--
Copyright 2013-2019 High Fidelity, Inc.
Copyright 2020-2021 Vircadia contributors
Copyright 2020-2026 Overte e.V.
SPDX-License-Identifier: Apache-2.0
-->

# Build macOS

*Last Updated on 2026-05-26*

Please read the [general build guide](BUILD.md) for information on dependencies required for all platforms.
This will include the necessary environment variables to customize your build. Only macOS specific instructions are found in this document.

## Prerequisites

### CMake, NPM and Conan

[Homebrew](https://brew.sh/) is an excellent package manager for macOS. It makes the installation of some Overte dependencies very simple.

```bash
brew install cmake npm conan
```

**Note:** You can also download alternative CMake versions from [Github](https://github.com/Kitware/CMake/releases) if needed.

### Qt5
#### system
While Conan can build Qt from source, we use the Homebrew package instead:
```bash
brew install qt@5
```

#### source
If you have Conan build Qt from source, the following extra dependencies are required:
```bash
pip3 install --user --break-system-packages html5lib
```
<!-- Qt Conan source package: Check for Python html5lib before attempting to build.
https://github.com/conan-io/conan-center-index/issues/27285
https://github.com/conan-io/conan-center-index/pull/29181 -->

**XCode** needs to be installed from the AppStore, which requires an Apple account.

### Prepare conan

The next step is setting up Conan.

Add the Overte remote to Conan
```bash
conan remote add overte https://artifactory.overte.org/artifactory/api/conan/overte -f
```

## Compiling

If you installed the Qt5 **system** package as instructed above, you need to add its path to the environment as instructed by Homebrew:
```bash
export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"
```
If you *only* need Qt5 on your machine (for example if you only develop Overte), you can make this change permanent as well:
```bash
echo 'export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"' >> ~/.zshrc
```

Install the dependencies with conan
```bash
cd overte
conan install . -s build_type=Release -b missing -pr:a="tools/conan-profiles/macos"
```

## Generate and Build

You can choose to use either Unix Makefiles, Ninja or Xcode.

### make or Ninja

Prepare makefiles:
```bash
cmake --preset conan-release
```
To use Ninja instead, append a `-G Ninja`. E.g.:
```bash
cmake --preset conan-release -G Ninja
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
**To be able to build, you need to turn off Xcode's `Use Shell Script Sandboxing`, as it results in commands too long for macOS.**
Open the `overte.xcodeproj` file, choose `ALL_BUILD` from the Product > Scheme menu (or target drop down), and click Run.

If the build completes successfully, you will have built targets for all components located in the `build/${target_name}/Debug` directories.

## Troubleshooting
- `error: unable to spawn process '/bin/sh' (Argument list too long)`
  Xcode sandboxes user scripts by putting all environment variables into the command, which then exceeds
  macOS's maximum command length.. As far as I can tell,
  there is no way to disable that through CMake (only inside the Xcode IDE).
  *Make sure you are using Ninja instead, by appending `-G Ninja` to the CMake configuration step.*
- Try updating CMake via `brew upgrade cmake`. Apple breaks things often and fast, so CMake requires updates pretty often.
