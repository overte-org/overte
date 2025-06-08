<!--
Copyright 2013-2019 High Fidelity, Inc.
Copyright 2019-2022 Vircadia contributors
Copyright 2021-2025 Overte e.V.
SPDX-License-Identifier: Apache-2.0
-->

# Build Linux

*Last Updated on 2025-05-21*

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


- QT 5:
```bash
sudo apt-get install -y qtbase5-dev \
                        qtbase5-private-dev \
                        qtwebengine5-dev \
                        qtwebengine5-dev-tools \
                        qtmultimedia5-dev \
                        libqt5opengl5-dev \
                        libqt5webchannel5-dev \
                        libqt5websockets5-dev \
                        qtxmlpatterns5-dev-tools \
                        qttools5-dev \
                        libqt5xmlpatterns5-dev \
                        libqt5svg5-dev \
                        qml-module-qtwebchannel \
                        qml-module-qtquick-controls \
                        qml-module-qtquick-controls2 \
                        qml-module-qt-labs-settings \
                        qml-module-qtquick-dialogs \
                        qml-module-qtwebengine
```


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

## Architecture support

If the build is intended to be packaged for distribution, the `OVERTE_CPU_ARCHITECTURE`
CMake variable needs to be set to an architecture specific value.

By default, it is set to `-march=native -mtune=native`, which yields builds optimized for a particular
machine, but these builds will not work on machines lacking same CPU instructions.

For packaging, it is recommended to set it to a different value, for example `-msse3`. This will help ensure that the build will run on all reasonably modern CPUs.

Setting `OVERTE_CPU_ARCHITECTURE` to an empty string will use the default compiler settings and yield maximum compatibility.


## Prepare conan

The next step is setting up conan

First, add the overte remote to conan
```bash
conan remote add overte https://artifactory.overte.org/artifactory/api/conan/overte -f
```

Optionally you can let conan automatically install the required system packages
```bash
echo "tools.system.package_manager:mode = install" >> ~/.conan2/global.conf
echo "tools.system.package_manager:sudo = True" >> ~/.conan2/global.conf
```

## Compiling

Prepare makefiles:
```bash
cmake . -DCMAKE_BUILD_TYPE=Release -Bbuild -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="cmake/conan_provider.cmake"
```

### Server

To compile the Domain server:
```bash
make domain-server assignment-client
```

*Note: For a server, it is not necessary to compile the Interface.*

### Interface

To compile the Interface client:
```bash
make interface
```

The commands above will compile with a single thread. If you have enough memory, you can decrease your build time using the `-j` flag. Since most x64 CPUs support two threads per core, this works out to CPU_COUNT*2. As an example, if you have a 2 core machine, you could use:
```bash
make -j4 interface
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

