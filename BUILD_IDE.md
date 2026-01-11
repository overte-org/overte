# CLion
While CLion is proprietary, it is free for non-commercial use. It supports pretty much everything,
is used by multiple team members, and runs on Linux, Windows, macOS. It is pretty much the same as Android Studio as well,
so it supports development for every Overte target platform.
You may want to use [Jetbrain's Toolbox app](https://www.jetbrains.com/toolbox-app/) to get automatic updates for CLion.

1. Generate the relevant CMake files for the platform you are on. E.g.: `conan install . -s build_type=Release -b missing -pr:b=default -of build -c tools.cmake.cmaketoolchain:generator="Ninja Multi-Config"`
   See the relevant BUILD_*.md for more details.

2. To work with different build types, run the `conan install` command again, replacing the `-s build_type=Release` with the relevant build type.
   E.g.: `-s build_type=Debug`. Valid build types are `Release`, `Debug`, and `RelWithDebInfo`.

3. Open CLion's "CMake Settings" and enable the relevant profile(s), e.g.: "Release", and press "OK".

   ![Open CMake settings](docs/CLion_CMake.png)
   ![Enable CMake profiles](docs/CLion_CMake_2.png)

   Now CLion should automatically switch to the CMake tab at the bottom and start configuring and generating.

   Keep in mind that only profiles with different build types can be enabled at the same time.
   Different CMake flags or environment variables cannot be enabled at the same time.
   Changing CMake flags or using a different category of profiles therefore requires you to disable the other profiles.
   Refer to the picture below, where the profiles are colored based on compatibility with each other:

   ![Enable CMake profile compatibility](docs/CLion_CMake_3.png "Compatibility of default profiles.")

3. Choose whatever target you want in the top right. Usually this is going to be "interface", which is Overte's client application.
   Keep in mind that the "interface" target is named "Overte" on macOS.

4. Now you at all set for building, running, and debugging.

## Tips
- `Enable QML language server` in CLion's settings to get warnings from qmllint. The binary is called `qmlls`.
  On Debian this is available in the `qt6-declarative-dev-tools` package and lives in `/usr/lib/qt6/bin/qmlls`.


