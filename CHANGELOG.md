<!--
Copyright 2022 Overte e.V.
SPDX-License-Identifier: MIT
-->

# Changelog
All notable changes to this project will be documented in this file.
This does not include changes to unrelated to the software or its packaging,
like documentaion or CI pipeline.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
This project does **not** adhere to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


<!-- ## [Unreleased] 2022.12.24 -->

## [2022.12.1] 2022.12.24
### Fixes
- Work around broken Nvidia driver (PR291)
- Fixed a bug in the glTF Serializer that caused the physics engine to crash (PR278)
- Fixed a crash when reading FST files with clothing information (PR270)
- Fixed a crash in the glTF Serializer (PR263)
	This fixes a crash in conjunction with ReadyPlayerMe avatars.
- Fixed stutters caused by the settings system (PR236)
	This was causing terrible stuttering on Linux systems using hard drives.
- Fixed Places app compatibility with Vircadia Metaverse version 2 (PR211)

### Changes
- Made the deadlock watchdog less spammy (PR288)
- Domain Server Metadata exporter by default is now disabled by default (PR247)
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
