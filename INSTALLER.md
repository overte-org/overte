<!--
Copyright 2013-2019 High Fidelity, Inc.
Copyright 2020-2021 Vircadia contributors
Copyright 2021-2022 Overte e.V.
SPDX-License-Identifier: Apache-2.0
-->

# Creating an Installer

*Last Updated on June 16, 2021*

Follow the [build guide](BUILD.md) to figure out how to build Overte for your platform.

During generation, CMake should produce an `install` target and a `package` target.

The `install` target will copy the Overte targets and their dependencies to your `CMAKE_INSTALL_PREFIX`.  
This variable is set by the `project(hifi)` command in `CMakeLists.txt` to `C:/Program Files/hifi` and stored in `build/CMakeCache.txt`

## Packaging

To produce an installer, run the `package` target. However you will want to follow the steps specific to your platform below.

### Windows

#### Prerequisites

To produce an executable installer on Windows, the following are required:

1. [7-zip](<https://www.7-zip.org/download.html>)  

1. [Nullsoft Scriptable Install System](http://nsis.sourceforge.net/Download) - 3.04  
  Install using defaults (will install to `C:\Program Files (x86)\NSIS`)
1. [UAC Plug-in for Nullsoft](http://nsis.sourceforge.net/UAC_plug-in) - 0.2.4c  
    1. Extract Zip
    1. Copy `UAC.nsh` to `C:\Program Files (x86)\NSIS\Include\`
    1. Copy `Plugins\x86-ansi\UAC.dll` to `C:\Program Files (x86)\NSIS\Plugins\x86-ansi\`
    1. Copy `Plugins\x86-unicode\UAC.dll` to `C:\Program Files (x86)\NSIS\Plugins\x86-unicode\`
1. [nsProcess Plug-in for Nullsoft](http://nsis.sourceforge.net/NsProcess_plugin) - 1.6 (use the link marked **nsProcess_1_6.7z**)
    1. Extract Zip
    1. Copy `Include\nsProcess.nsh` to `C:\Program Files (x86)\NSIS\Include\`
    1. Copy `Plugins\nsProcess.dll` to `C:\Program Files (x86)\NSIS\Plugins\x86-ansi\`
    1. Copy `Plugins\nsProcessW.dll` to `C:\Program Files (x86)\NSIS\Plugins\x86-unicode\`

1. [InetC Plug-in for Nullsoft](http://nsis.sourceforge.net/Inetc_plug-in) - 1.0
    1. Extract Zip
    1. Copy `Plugin\x86-ansi\InetC.dll` to `C:\Program Files (x86)\NSIS\Plugins\x86-ansi\`
    1. Copy `Plugin\x86-unicode\InetC.dll` to `C:\Program Files (x86)\NSIS\Plugins\x86-unicode\`
    
1. [NSISpcre Plug-in for Nullsoft](http://nsis.sourceforge.net/NSISpcre_plug-in) - 1.0
    1. Extract Zip
    1. Copy `NSISpre.nsh` to `C:\Program Files (x86)\NSIS\Include\`
    1. Copy `NSISpre.dll` to `C:\Program Files (x86)\NSIS\Plugins\x86-ansi\`

1. [nsisSlideshow Plug-in for Nullsoft](<http://wiz0u.free.fr/prog/nsisSlideshow/>) - 1.7
   1.  Extract Zip
   1.  Copy `bin\nsisSlideshow.dll` to `C:\Program Files (x86)\NSIS\Plugins\x86-ansi\`
   1.  Copy `bin\nsisSlideshowW.dll` to `C:\Program Files (x86)\NSIS\Plugins\x86-unicode\`

1. [Nsisunz plug-in for Nullsoft](http://nsis.sourceforge.net/Nsisunz_plug-in)
   1.  Download both Zips and unzip
   1.  Copy `nsisunz\Release\nsisunz.dll` to `C:\Program Files (x86)\NSIS\Plugins\x86-ansi\`
   1.  Copy `NSISunzU\Plugin unicode\nsisunz.dll` to `C:\Program Files (x86)\NSIS\Plugins\x86-unicode\`

1. [ApplicationID plug-in for Nullsoft]() - 1.0
   1.  Download [`Pre-built DLLs`](<https://github.com/connectiblutz/NSIS-ApplicationID/releases/download/1.1/NSIS-ApplicationID.zip>)
   1.  Extract Zip
   1.  Copy `Release\ApplicationID.dll` to `C:\Program Files (x86)\NSIS\Plugins\x86-ansi\`
   1.  Copy `ReleaseUnicode\ApplicationID.dll` to `C:\Program Files (x86)\NSIS\Plugins\x86-unicode\`

1. [Node.JS and NPM](https://nodejs.org/en/download/)
    1.  Install version 10.15.0 LTS (or greater)
    
#### Code Signing (optional)

For code signing to work, you will need to set the `HF_PFX_FILE` and `HF_PFX_PASSPHRASE` environment variables to be present during CMake runtime and globally as we proceed to package the installer.

#### Creating the Installer
    
1.  Perform a clean cmake from a new terminal.
1.  Open the `overte.sln` solution with elevated (administrator) permissions on Visual Studio and select the **Release** configuration.
1.  Build the solution.
1.  Build `packaged-server-console-npm-install` (found under **hidden/Server Console**)
1.  Build `packaged-server-console` (found under **Server Console**)  
    This will add 2 folders to `build\server-console\` -  
    `server-console-win32-x64` and `x64`
1.  Build CMakeTargets->PACKAGE   
    The installer is now available in `build\_CPack_Packages\win64\NSIS`

#### Create an MSIX Package

1. Get the 'MSIX Packaging Tool' from the Windows Store.
2. Run the process to create a new MSIX package from an existing .exe or .msi installer. This process will allow you to install Overte with the usual installer, however it will monitor changes to the computer to replicate the functionality in the MSIX Package. Therefore, you will want to avoid doing anything else on your computer during this process.
3. Be sure to select no shortcuts and install only the Overte Interface.
4. When asked for "Entry" points, select only the Interface entry and not the uninstaller. This is because the MSIX package is uninstalled by Windows itself. If for some reason the uninstaller shows up anyway, you can edit the manifest to manually remove it from view even if the uninstaller is present in the package. This is necessary to uplaod to the Windows Store.
5. Once completed, you can sign the package with this application or with other tools such as 'MSIX Hero'. It must be signed with a local certificate to test, and with a proper certificate to distribute.
6. If uploading to the Windows Store, you will have to ensure all your manifest info including publisher information matches what is registered with your Microsoft Developer account for Windows. You will see these errors and the expected values when validating it.

#### FAQ

1. **Problem:** Failure to open a file. ```File: failed opening file "\FOLDERSHARE\XYZSRelease\...\Credits.rtf" Error in script "C:\TFS\XYZProject\Releases\NullsoftInstaller\XYZWin7Installer.nsi" on line 77 -- aborting creation process```
    1. **Cause:** The complete path (current directory + relative path) has to be < 260 characters to any of the relevant files.
    1. **Solution:** Move your build and packaging folder as high up in the drive as possible to prevent an overage.

### MacOS

1. Ensure you have all the prerequisites fulfilled from the [MacOS Build Guide](BUILD_OSX.md).
2. Perform a clean CMake in your build folder. e.g.
    ```bash
    BUILD_GLOBAL_SERVICES=STABLE USE_STABLE_GLOBAL_SERVICES=1 RELEASE_BUILD=PRODUCTION BUILD_NUMBER="Insert Build Identifier here e.g. short hash of your last Git commit" STABLE_BUILD=1 PRODUCTION_BUILD=1 RELEASE_NUMBER="Insert Release Version Here e.g. 1.1.0" RELEASE_TYPE=PRODUCTION cmake -DCMAKE_OSX_SYSROOT="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk" -DCLIENT_ONLY=1 -DCMAKE_OSX_DEPLOYMENT_TARGET=10.12 -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl -DOSX_SDK=10.12  ..
    ```
3. Pick a method to build and package your release.

#### Option A: Use Xcode GUI

1. Perform a Release build of ALL_BUILD
2. Perform a Release build of `packaged-server-console` 
     This will add a folder to `build\server-console\` -  
     Sandbox-darwin-x64
3. Perform a Release build of `package`
      Installer is now available in `build/_CPack_Packages/Darwin/DragNDrop`

#### Option B: Use Terminal

1. Navigate to your build folder with your terminal.
2. `make -j4`, you can change the number to match the number of threads you would like to use.
3. `make package` to create the package.

### Linux

#### Server

##### Debian package

###### Building Overte server Debian packages using Docker

1. Build Docker image as instructed in the relevant Dockerfile in [tools/ci-scripts/deb_package/](tools/ci-scripts/deb_package/)

2. Create/Start container
	Example: `docker run -v $(pwd)/../../..:/overte -it overte/overte-server-build:0.1.2-debian-11-amd64`

3. Prepare build environment
```bash
cd overte
mkdir build
cd build
rm -rf *
```
Add `PRODUCTION_BUILD=1` to below command for release and release candidate builds.
```bash
OVERTE_USE_SYSTEM_QT=true cmake .. -DOVERTE_CPU_ARCHITECTURE=-msse3 -DVCPKG_BUILD_TYPE=release -DSERVER_ONLY=true -DBUILD_TOOLS=true
```

4. Build
```bash
make domain-server assignment-client oven -j$(nproc)
```

5. Create Debian package
```bash
cd ../pkg-scripts
```
For Debian 11 and Ubuntu 22.04:
```bash
OVERTE_USE_SYSTEM_QT="true" DEBVERSION="1-experimental-debian-11" DEBEMAIL="julian.gro@overte.org" DEBFULLNAME="Julian Groß" ./make-deb-server
```
For Ubuntu 18.04 and 20.04:
```bash
DEBVERSION="1-experimental-ubuntu-18.04" DEBEMAIL="julian.gro@overte.org" DEBFULLNAME="Julian Groß" ./make-deb-server
```

##### RPM package

###### Building Overte server RPM packages using Docker

1. Build Docker image as instructed in the relevant Dockerfile in [tools/ci-scripts/rpm_package/](tools/ci-scripts/rpm_package/)

2. Create/Start container
	Example: `docker run -v $(pwd)/../../..:/overte -it overte/overte-server-build:0.1.2-fedora-36-amd64`

3. Prepare build environment
```bash
cd overte
mkdir build
cd build
rm -rf *
```
Add `PRODUCTION_BUILD=1` to below command for release and release candidate builds.
```bash
OVERTE_USE_SYSTEM_QT=true cmake .. -DOVERTE_CPU_ARCHITECTURE=-msse3 -DVCPKG_BUILD_TYPE=release -DSERVER_ONLY=true -DBUILD_TOOLS=true
```

4. Build
```bash
make domain-server assignment-client oven -j$(nproc)
```

5. Create RPM package
```bash
cd ../pkg-scripts
```
```bash
OVERTE_USE_SYSTEM_QT="true" RPMVERSION="1.experimental" ./make-rpm-server
```
