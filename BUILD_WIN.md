<!--
Copyright 2013-2019 High Fidelity, Inc.
Copyright 2019-2021 Vircadia contributors
Copyright 2021-2025 Overte e.V.
SPDX-License-Identifier: Apache-2.0
-->

# Build Windows

*Last Updated on 2025-03-17*

This is a stand-alone guide for creating your first Overte build for Windows 64-bit.

Note: We are now using Visual Studio 2019 and Qt 5.15.x.
If you are upgrading from previous versions, do a clean uninstall of those versions before going through this guide.

**Note: The prerequisites will require about 10 GB of space on your drive. You will also need a system with at least 8GB of main memory.**

## Step 1. Visual Studio & Python 3.x

If you don't have Community or Professional edition of Visual Studio 2019, download [Visual Studio Community 2019](https://visualstudio.microsoft.com/vs/). If you have Visual Studio 2017, you need to download Visual Studio 2019.

When selecting components, check "Desktop development with C++".

If you do not already have a Python 3.x development environment installed and want to install it with Visual Studio, check "Python Development". If you already have Visual Studio installed and need to add Python, open the "Add or remove programs" control panel and find the "Microsoft Visual Studio Installer". Select it and click "Modify". In the installer, select "Modify" again, then check "Python Development" and allow the installer to apply the changes.

### Visual Studio 2019

On the right on the Summary toolbar, select the following components.

* MSVC v142 - VS 2019 C++ X64/x86 build tools
* MSVC v141 - VS 2017 C++ x64/x86 build tools
* MSVC v140 - VS 2015 C++ build tools (v14.00)

## Step 1a. Alternate Python

If you do not wish to use the Python installation bundled with Visual Studio, you can download the installer from [here](https://www.python.org/downloads/). Ensure that you get version 3.6.6 or higher.

## Step 2. Python Dependencies

In an administrator command-line that can access Python's pip you will need to run the following command:

`pip install distro`

If you do not use an administrator command-line, you will get errors.

## Step 3. Installing Conan

Download and install Conan from the [Conan website](https://conan.io/downloads).
Next, add the Overte remote to Conan:
```bash
conan remote add overte https://artifactory.overte.org/artifactory/api/conan/overte
```

## Step 4. Installing CMake

Download and install CMake version 3.15 or higher.

Download the file named cmake-[version]-windows-x86_64.msi Installer from the [CMake Website](https://cmake.org/download/). During installation, make sure to check "Add CMake to system PATH for all users" when prompted.

## Step 5. (Optional) Node.JS and NPM

Install the latest LTS version of [Node.JS and NPM](<https://nodejs.org/en/download/>).
This is required to build the server-console and jsdoc, and for JavaScript console autocompletion.

## Step 6. (Optional) Create conan environment variable
In the next step, you will use conan to install the dependencies required to build Overte. By default, conan will build and install the dependencies in `<username>/.conan2`.
If you want to change that location you can create a `CONAN_HOME` environment variable linked to a directory somewhere on your machine.

To create this variable:
* Navigate to 'Edit the System Environment Variables' Through the Start menu.
* Click on 'Environment Variables'
* Select 'New'
* Set "Variable name" to `CONAN_HOME`
* Set "Variable value" to any directory that you have control over.

## Step 7. Running CMake to Generate Build Files

These instructions only apply to Visual Studio 2019.

### Automatic

There is a batch file to automatically run the commands below for ease of use.

`winprepareVS19.bat`

### Manual

Run the Command Prompt from Start and run the following commands:

```bash
cd "%OVERTE_DIR%"
cmake . -G"Visual Studio 16 2019" -Bbuild -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="cmake/conan_provider.cmake"
```

Where `%OVERTE_DIR%` is the directory for the Overte repository.

Note: After running Conan it is recommended to run `conan cache clean "*" -sbd` to clean the build folders created by Conan, saving disk space.

## Step 8. Making a Build

Open `%OVERTE_DIR%\build\overte.sln` using Visual Studio.

Change the Solution Configuration (menu ribbon under the menu bar, next to the green play button) from "Debug" to "Release" for best performance.

Run from the menu bar `Build > Build Solution`.

## Step 9. Testing Interface

Create another environment variable (see Step #3)
* Set "Variable name": `_NO_DEBUG_HEAP`
* Set "Variable value": `1`

Restart Visual Studio again.

In Visual Studio, right-click "interface" under the Apps folder in Solution Explorer and select "Set as Startup Project". Run from the menu bar `Debug > Start Debugging`.

Now, you should have a full build of Overte and be able to run the Interface using Visual Studio.

Note: You can also run Interface by launching it from command line or File Explorer from `%OVERTE_DIR%\build\interface\Release\interface.exe`

# Troubleshooting

For any problems after Step #7, first try this:
* Delete your locally cloned copy of the Overte repository
* Restart your computer
* Redownload the [repository](https://github.com/overte-org/overte)
* Restart directions from Step #7

## CMake gives you the same error message repeatedly after the build fails

Remove `CMakeCache.txt` found in the `%OVERTE_DIR%\build` directory.
