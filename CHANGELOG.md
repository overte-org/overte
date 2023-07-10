<!--
Copyright 2022-2023 Overte e.V.
SPDX-License-Identifier: Apache-2.0
-->

# Changelog
All notable changes to this project will be documented in this file.
This does not include changes to unrelated to the software or its packaging,
like documentaion or CI pipeline.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
This project does **not** adhere to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


<!-- ## [Unreleased] 2023.07.09 -->
<!-- ## [2023.07.1] 2023.07.09 -->

<!--
### Misc
- Updated the Unity Avatar Exporter and added Linux support
- Added Linux support to the Unity Avatar Exporter
-->

### Fixes
- Fixed color conversion for glTF material colors (PR307)
- Fixed a possible crash in Mesh::map (PR392)
- Fixed multiple memory corruption issues that were causing frequent crashes (PR185)
	Most notably this gets rid of a frequent Create App related crash.
- Hide private methods and QObject signals from script engine (PR444)
- Fixed a regression that broke resetting the settings (PR445)
- Fixed deadlock on start on Debian Testing (PR185)
- Fixed Windows 11 showing up as Windows 10 in logs (PR448)
- Fixed Metallic values on FBX models created by Blender 2.79 or earlier (PR463)
- Fixed laser pointers being rendered below overlays (PR490)
- Fixed angle text sometimes being occluded when using Create app (PR498)
- Hugely improved Create app performance in Domains with many entities (PR498)
- Fixed an issue that could cause laser pointers to rapidly flash (PR495)

### Changes
- Replaced Vircadia Metaverse Server with a testing server as federation default (PR330)
- An empty audio device list now throws a warning instead of just a debug message (PR347)
- Increased the maximum log file size from 512 kiB to 10 MiB (PR342,PR513)
- Decreased the amount of retained log files from 100 to 20 (PR342)
- Pressing the Return key with the the address/search bar in the Places App selected now navigates you to that address (PR403)
- Replaced QT Script with V8 scripting engine (PR185,PR507)
	This is a huge change under the hood, which ended up fixing a lot of issues.
	Since the new scripting engine does not behave exactly the same as the old one,
	some scripts might need fixing. The new scripting engine is especially picky when it comes to undefined behaviour.
	Most notably "use strict" is actually strict now and won't allow you to use variables without defining them first.
- Silenced ForceHtmlAudioOutputDeviceUpdate log message (PR473)
- Improved crash reporting functionality (PR480,PR484)
	Interface will ask if future crashes should be reported automatically when it detects that it crashed on last run.
	It will also ask once in case of a non-stable build.
- Changed the VR overlay to only recenter when moving (PR478)
- Added a workaround that prevents most users from needing to press down on the thumbstick to move (PR481,PR512)

### Additions
- Added option to graphics menu for choosing which screen to use for full screen mode (PR302)
- file URLs can now be navigated to using the Places App (PR397)
- Added IME support in Desktop mode (PR431)
	This allows typing in languages like Japanese or Chinese that make use of an IME.
- Added vertical Field Of View setting to graphics menu (PR465)
- Added crash reporting to the Domain server, Assignment client, and Oven (PR482)

### Removals
- Removed outdated Inventory and Marketplace options from Wearables UI (PR303)
- Removed outdated Beacon system (PR327)
- Removed long deprecated styles-uit and controls-uit QML modules (PR380)
- Removed outdated Marketplace and Wallet code (PR381,PR477,PR487)

### Build system
- Fixed error in configuration step on some rolling release Linux distributions (PR301)
- Removed executable permissions from files that shouldn't have them (PR349)
- Added QML import path (PR379)
- Fixed building on GCC 13 (PR385)
- Fixed a bunch of warnings on C++20 (PR385)
- Updated TBB dependency from version 2019_U8-1 to 2021.5.0 (PR412)
- Fixed NVTT compilation on Visual Studio 2022 (PR374)
- Disabled libOVR on MSVC 2022 (PR430)
- Added Qt 5.15.9 package for aarch64 Ubuntu 20.04 (PR409)
- Fixed build error on aarch64 (PR409)
- Replaced QT Script with V8/libnode (PR185,PR409,PR443)
- Updated Qt on Windows to 5.15.10 with KDE patches (PR448)
- Updated included OpenSSL to 3.0.5 (PR448)



## [2022.12.1] 2022.12.24
### Fixes
- Secondary camera now uses same anti-aliasing setting as primary camera (PR294)
- Work around broken Nvidia driver (PR291)
- Fixed a bug in the glTF Serializer that caused the physics engine to crash (PR278)
- Fixed a crash when reading FST files with clothing information (PR270)
- Fixed a crash in the glTF Serializer (PR263)
	This fixes a crash in conjunction with ReadyPlayerMe avatars.
- Fixed stutters caused by the settings system (PR236)
	This was causing terrible stuttering on Linux systems using hard drives.
- Fixed Places app compatibility with Vircadia Metaverse version 2 (PR211)

### Changes
- Disable anti-aliasing by default (PR294)
- Made the deadlock watchdog less spammy (PR288)
- Domain Server Metadata exporter is now disabled by default (PR247)
	This avoids port conflicts when running multiple Domain Servers on one machine.
- Improved the notification system (PR190,PR189)
	Notifications can be closed by just clicking them now.
	Snapshot notifications can now be turned off.
	VR notifications are now unlit and have been slightly moved.
	The notification system is generally more stable.

### Additions
- Added partial Blender FBX metallic support (PR221)
- Added tooltips for PolyVox/Voxel properties to Create App (PR244)
- Added a setting to disable snapshot notifications (PR189)
- Added a setting to switch between screenshot formats (PR134)

### Removals
-

### Build system
- Fixed "may be used uninitialized" warning for blendtime (PR269)
- Updated SPIRV-Cross to sdk-1.3.231.1 (PR271)
- Started working towards REUSE compliance (PR262)
- Fixed a bunch of test warnings (PR268)
- Added texture benchmark test (PR84)
- Updated SPIR-V to 2022.4 (PR267)
- Added basic Audio and Codec tests (PR177)


## [2022.09.1] 2022.10.06
### Fixes
- Fixed missing buttons in domain server UI (PR209)
- Fixed misbehaving entity selection in VR (PR191)
- Fixed unintended voxel editing when using the tablet (PR191)
- Fixed buggy voxel synchronization (PR184)
- Added a timeout to the Places app in case a directory server is unreachable (PR183)
- Fixed OBJ-Serializer using wrong decimal separator on some systems (PR172)
- Fixed "disable-displays" and "disable-inputs" command line arguments (PR169)
- Fixed warnings during Create app usage (PR161)
- Fixed various issues with voxels (PR140)
- Improved stability of the server software (PR129)
- Fixed broken More App permission (PR108)
- Fixed Chromium related warnings (PR103)
- Fixed "replaceAvatarURL" command line argument (PR99)
- Fixed warning in simplifiedNametags (PR94)
- Fixed issues with command line arguments (PR66)
- Fixed a lot of miscellaneous warnings (PR31)

### Changes
- Increased default voxel resolution (PR191)
- Replaced non-free Graphik font with Fira Sans (PR155)
- Updated Qt for Windows, and Linux (PR146,PR125)
- Changed VR Keyboard appearance (PR147)
- Moved to new URL for hosted assets and did some rebranding (PR149)
- Changed default control scheme in VR to analog (PR144)
- Changed default domain-server networking setting (PR143)
- Changed default screenshot format to PNG (PR112,PR120)
- Changed "Wrote KTX" message to debug (PR100)
- Show filename when a texture fails to load (PR97)
- Improved More app (PR80)
- Improved gamepad control scheme (PR55)
- Changed default ICE server (PR34)
- Rebranding (PR34)
- Updated the tutorial (PR27,PR45,PR53)
- Changed default Metaverse server (PR26)
- Changed default More app repository (PR8)

### Additions
- Added Material Data Assistant to Create App (PR131)
- Added body tracking support for Windows Mixed Reality (PR111)
- Added Journald logging support (PR104)
	This is enabled by default on the server software.
- Added log breakpoint system for debugging (PR95)
- Added version of Qt WebEngine and Chromium to About screen (PR93)
- Added user interface for voxels to Create app (PR60)

### Removals
- Removed unused Inventory app

### Build system
- Added helper scripts for building server packages (PR174)
- Disabled VCPKG logs being removed after completion (PR162)
- Updated VCPKG (PR162)
	This bumps the minimum CMake version to 3.21
- Updated NVTT (PR165)
- Added Ubuntu 20.04 Qt pre-built package (PR159)
- Changed a lot of Vircadia optons to Overte ones (PR149)
- Updated to zlib 1.2.12 (PR123)
- Moved dependency hosting (PR121)
- Added Journald as optional dependency (PR104)
- Fixed trivial CMake warning (PR102)
- Fixed a lot of warnings during build (PR86)
- Fixed WebRTC OpenSSL 3 linking issue (PR68)
- Fixed SDL2 linking issues on Wayland (PR47)


## [2022.02.1] 2022-02-23
### Additions
- Allow switching Metaverse servers (PR2)

### Changes
- Rebranding (PR13)
- Changed default Metaverse server (PR5)
- Replaced the Explore app with a new Places app (PR3)

### Removals
- Removed proprietary HiFi audio codec (PR1)
