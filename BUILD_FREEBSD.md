<!--
Copyright 2013-2019 High Fidelity, Inc.
Copyright 2019-2022 Vircadia contributors
Copyright 2021-2026 Overte e.V.
SPDX-License-Identifier: Apache-2.0
-->

# Build FreeBSD

*Last Updated on 2026-03-19*

Please read the [general build guide](BUILD.md) for information on dependencies required for all platforms. Only FreeBSD specific instructions are found in this file.

## Install build tools:

First update the system packages:
```sh
pkg upgrade
```

Install dependencies
```sh
pkg install git gmake cmake conan
```

## Extra dependencies to compile Interface on a server
Install the following:
```sh
pkg install qt5-core qt5-network qt5-testlib qt5-websockets qt5-gui qt5-widgets qt5-concurrent qt5-quickcontrols2 qt5-multimedia qt5-webchannel qt5-webengine qt5-xml qt5-xmlpatterns qt5-svg
```

Install Python 3:
```sh
pkg install python3
```

Install Node.js as it is required to build the jsdoc documentation:
```sh
pkg install node
```

## Get code and checkout the branch you need

Clone this repository:
```sh
git clone https://github.com/overte-org/overte.git
```

Then checkout the master branch with:
```sh
git checkout master
```

If you need a different branch, you can get a list of all tags with:
```sh
git fetch --tags
git tag
```

## Prepare conan

The next step is setting up conan

First, create a conan profile
```sh
conan profile detect --force
```

Next, add the overte remote to conan
```sh
conan remote add overte https://artifactory.overte.org/artifactory/api/conan/overte -f
```

## Compiling

Install the dependencies with conan
```sh
cd overte
conan install . -s build_type=Release -b missing -pr:a=tools/conan-profiles/freebsd -of build
```

If you want to build Debug or RelWithDebInfo versions, change the `build_type` to `Debug` or `RelWithDebInfo` and run the command again. E.g.:
```sh
conan install . -s build_type=Debug -b missing -pr:a=tools/conan-profiles/freebsd -of build
```

Prepare makefiles:
```sh
cmake --preset conan-default
```

### Server

To compile the Domain server:
```sh
cmake --build --preset conan-release domain-server assignment-client
```

*Note: For a server, it is not necessary to compile the Interface.*

### Interface

To compile the Interface client:
```sh
cmake --build --preset conan-release interface
```

## Running the software

### Domain server

Running Domain server:
```sh
./domain-server/domain-server
```

### Assignment clients

Running assignment client:
```sh
./assignment-client/assignment-client -n 6
```

### Interface

Running Interface:
```sh
./interface/interface
```

Go to "localhost" in the running Interface to visit your newly launched Domain server.

