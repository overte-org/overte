<!--
Copyright 2013-2019 High Fidelity, Inc.
Copyright 2019-2022 Vircadia contributors
Copyright 2021-2025 Overte e.V.
SPDX-License-Identifier: Apache-2.0
-->

# Build Linux

*Last Updated on 2026-01-11*

Please read the [general build guide](BUILD.md) for information on dependencies required for all platforms. Only Linux specific instructions are found in this file.

~~You can use the [Overte Builder](https://github.com/overte-org/overte-builder) to build on Linux more easily. Alternatively, you can follow the manual steps below.~~ (Currently outdated.)

This documentation assumes that you are running our current target distribution, which is currently Ubuntu 22.04. The target distribution is usually the latest Ubuntu LTS still receiving standard support, though we may upgrade a little sooner if we require certain newer packages. Ubuntu version numbers are date codes and standard support is 5 years, meaning that Ubuntu 22.04 leaves standard support after around 2027-04.

## Install build tools:

-  First update the package cache and your system:
```bash
sudo apt update
sudo apt upgrade
```

-  Install git and g++
```bash
sudo apt install git g++
```

-  Install CMake
We require a newer CMake version than 3.22.1, which is shipped in Ubuntu 22.04, so we install CMake packages provided by upstream here: https://apt.kitware.com/

-  Install Conan
Get the Conan "Ubuntu / Debian installer" from https://conan.io/downloads and install it using:
```bash
sudo apt install ./conan-*.deb
```
Verify Conan was installed by running `conan --version`.

## Install build dependencies:
Most dependencies will be automatically installed by Conan. This section only lists dependencies which might not be handled by Conan.

- OpenGL:
```bash
sudo apt-get install libgl1-mesa-dev -y
```
Verify OpenGL:
  - First install mesa-utils with the command `sudo apt install mesa-utils -y`.
  - Then run `glxinfo | grep "OpenGL version"`.

## Extra dependencies to compile Interface on a server
- Install the following:
```bash
sudo apt install libpulse0 libnss3 libnspr4 libfontconfig1 libxcursor1 libxcomposite1 libxtst6 libxslt1.1
```

-  Misc dependencies:
```bash
sudo apt install libasound2 libxmu-dev libxi-dev freeglut3-dev libasound2-dev libjack0 libjack-dev libxrandr-dev libudev-dev libssl-dev zlib1g-dev
```

-  Install Python 3 and required packages:
```bash
sudo apt install python python3 python3-distro
```

-  Install Node.js as it is required to build the jsdoc documentation:
```bash
sudo apt install nodejs
```

## Get code and checkout the branch you need

Clone this repository:
```bash
git clone https://github.com/overte-org/overte.git
```

Then checkout the master branch with:
```bash
git checkout master
```

If you need a different branch, you can get a list of all tags with:
```bash
git fetch --tags
git tag
```

## Prepare conan

The next step is setting up conan

First, create a conan profile
```bash
conan profile detect --force
```

Next, add the overte remote to conan
```bash
conan remote add overte https://artifactory.overte.org/artifactory/api/conan/overte -f
```

Let conan automatically install the required system packages
```bash
echo "tools.system.package_manager:mode = install" >> ~/.conan2/global.conf
echo "tools.system.package_manager:sudo = True" >> ~/.conan2/global.conf
```
If you don't do this, Conan will still complain if it notices system packages being missing, so you can manually install them.

## Compiling

Install the dependencies with conan
```bash
cd overte
conan install . -s build_type=Release -b missing -pr:b=default -of build -c tools.cmake.cmaketoolchain:generator="Ninja Multi-Config"
```

On systems with GCC 15 additional parameter is needed:
```bash
cd overte
conan install . -s build_type=Release -b missing -pr:b=default -of build -c tools.cmake.cmaketoolchain:generator="Ninja Multi-Config" -c tools.build:cxxflags="['-include', 'cstdint']"
```

If you want to build Debug or RelWithDebInfo versions, change the `build_type` to `Debug` or `RelWithDebInfo` and run the command again. E.g.:
```bash
conan install . -s build_type=Debug -b missing -pr:b=default -of build -c tools.cmake.cmaketoolchain:generator="Ninja Multi-Config"
```

Prepare ninja files:
```bash
cmake --preset conan-default
```

### Server

To compile the Domain server:
```bash
cmake --build --preset conan-release domain-server assignment-client
```

*Note: For a server, it is not necessary to compile the Interface.*

### Interface

To compile the Interface client:
```bash
cmake --build --preset conan-release interface
```

## Running the software

### Domain server

Running Domain server:
```bash
./domain-server/domain-server
```

### Assignment clients

Running assignment client:
```bash
./assignment-client/assignment-client -n 6
```

### Interface

Running Interface:
```bash
./interface/interface
```

Go to "localhost" in the running Interface to visit your newly launched Domain server.
