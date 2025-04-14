//
//  WebRTC.h
//  libraries/shared/src/shared/
//
//  Copyright 2019 High Fidelity, Inc.
//  Copyright 2021 Vircadia contributors.
//  Copyright 2023-2025 Overte e.V.
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
#elif defined(Q_OS_WIN)
#  define WEBRTC_AUDIO 1
// We use PulseAudio's webrtc-audio-processing, which doesn't include Data Channels.
// Data Channels were only used for Vircadia's web client.
// We should replace Google WebRTC with libdatachannel if we ever resurrect it.
// #  define WEBRTC_DATA_CHANNELS 1
#  define WEBRTC_WIN 1
#  define NOMINMAX 1  // Windows.h
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN 1
#  endif
#elif defined(Q_OS_ANDROID)
#  define WEBRTC_AUDIO 1
#elif defined(Q_OS_LINUX)
#  ifndef DISABLE_WEBRTC
#    define WEBRTC_AUDIO 1
// #  define WEBRTC_DATA_CHANNELS 1
#  endif
#endif

#endif // hifi_WebRTC_h
