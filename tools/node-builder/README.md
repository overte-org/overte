# Copyright 2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

# General
This document describes the process to build Node 18.14.2 for usage as scripting engine..

Reference: https://github.com/nodejs/node/blob/main/BUILDING.md

## Requirements
### Windows
1. git
2. nasm-2.16.01-installer-x64.exe

### Linux
1. git

### Mac
TODO

## Build Process


### General
The build is performed in source.
Build products are installed to the node-install folder.
Before running configure, make sure that the node-build folder is empty.


### Windows
Make sure that the directory you are using to build Node is not deeply nested.  It is quite possible to run into the windows MAX_PATH limit when building Node.  For example: `c:\msys64\home\ajt\code\hifi\tools\node-builder\node-build` is too long.  `c:\n\node-build\` is a better choice.

Build:
'.\vcbuild.bat release package dll nonpm'

Rename the installation directory to 'node-build'

After building copy 'deps\v8\include\cppgc' and 'deps\v8\include\libplatform' to 'node-build\include'.
Copy 'libnode.lib' and 'v8_libplatform.lib' to 'node-build'

#### Preparing source files
Get the source:
`git clone --recursive https://github.com/nodejs/node.git -b v18.14.2 --single-branch`

#### Configuring
TODO

#### Make
TODO

#### Uploading

Create a xz tar file called node-install-18.14.2-windows-release.tar.gz from the node-install folder.

Using 7-Zip:
* `cd` to the *qt5* folder.
* `7z a -ttar qnode-install-18.14.2-windows-release.tar node-install`
* `7z a -txz node-install-18.14.2-windows-release.tar.xz node-install-18.14.2-windows-release.tar`

Upload node-install-18.14.2-windows.tar.xz to build-deps.overte.org, under the dependencies/node directory.


### Linux
#### Preparing source files
```bash
git clone --recursive https://github.com/nodejs/node.git -b v18.14.2 --single-branch
```

#### Configuring
```bash
mkdir node-install
```

release:
```bash
cd node
./configure --gdb --shared --prefix=../node-install/
```

debug:
```bash
cd node
./configure --gdb --shared --debug --debug-lib --v8-with-dchecks --v8-enable-object-print --prefix=../node-install/
```

#### Make
Replace `4` with the amount of threads you want to use. Keep in mind that the Node build process uses a lot of memory. It is recommended to have at least 1.2 GiB per thread.
```bash
make -j4
```

Now Node can be installed to node-install:
```bash
make -j4 install
```

#### Uploading
1.  Tar and xz node-install to create the package. Replace `ubuntu-18.04` with the relevant system and `amd64` with the relevant architecture.
```bash
tar -Jcvf node-install-18.14.2-ubuntu-18.04-amd64-release.tar.xz node-install
```
2.  Upload node-install-18.14.2-ubuntu-18.04-amd64-release.tar.xz to https://build-deps.overte.org/dependencies/node/



### Mac
#### Preparing source files
```bash
git clone --recursive https://github.com/nodejs/node.git -b v18.14.2 --single-branch
```

#### Configuring
TODO

#### Make
TODO

#### Uploading
```bash
tar -Jcvf node-install-18.14.2-macOSXSDK10.14-macos-release.tar.xz node-install
```
Upload node-install-18.14.2-macOSXSDK10.14-macos-release.tar.xz to build-deps.overte.org, under the dependencies/node directory.
