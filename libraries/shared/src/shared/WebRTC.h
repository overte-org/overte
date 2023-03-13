//
//  WebRTC.h
//  libraries/shared/src/shared/
//
//  Copyright 2019 High Fidelity, Inc.
//  Copyright 2021 Vircadia contributors.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_WebRTC_h
#define hifi_WebRTC_h

#ifndef QSYSTEMDETECTION_H
#include <qsystemdetection.h>
#endif

// WEBRTC_AUDIO: WebRTC audio features, e.g., echo canceling.
// WEBRTC_DATA_CHANNELS: WebRTC client-server connections in parallel with UDP.

#if defined(Q_OS_MAC)
#  define WEBRTC_AUDIO 1
#  define WEBRTC_POSIX 1
#  define WEBRTC_LEGACY 1
#elif defined(Q_OS_WIN)
#  define WEBRTC_AUDIO 1
#  define WEBRTC_DATA_CHANNELS 1
#  define WEBRTC_WIN 1
#  define NOMINMAX 1
#  define WIN32_LEAN_AND_MEAN 1
#elif defined(Q_OS_ANDROID)
// I don't yet have a working libwebrtc for android
// #  define WEBRTC_AUDIO 1
// #  define WEBRTC_POSIX 1
// #  define WEBRTC_LEGACY 1
#elif defined(Q_OS_LINUX) && defined(Q_PROCESSOR_X86_64)
//V8TODO:
// WEBRTC_AUDIO causes:
// overte/libraries/audio-client/src/AudioClient.cpp:1258:36: runtime error: member call on address 0x61b000006980 which does not point to an object of type 'AudioProcessing'
// 0x61b000006980: note: object has invalid vptr
#  define WEBRTC_AUDIO 1
#  define WEBRTC_POSIX 1
//V8TODO: temporarily disabled, because linker failed for memory debugging
#  define WEBRTC_DATA_CHANNELS 1
#elif defined(Q_OS_LINUX) && defined(Q_PROCESSOR_ARM)
// WebRTC is basically impossible to build on aarch64 Linux.
// I am looking at https://gitlab.freedesktop.org/pulseaudio/webrtc-audio-processing for an alternative.
// #  define WEBRTC_AUDIO 1
// #  define WEBRTC_POSIX 1
// #  define WEBRTC_LEGACY 1
#endif

#endif // hifi_WebRTC_h
