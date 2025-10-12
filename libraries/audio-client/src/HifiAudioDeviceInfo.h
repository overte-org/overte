//
//  HifiAudioDeviceInfo.h
//  libraries/audio-client/src
//
//  Created by Amer Cerkic on 9/14/19.
//  Copyright 2019 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_audiodeviceinfo_h
#define hifi_audiodeviceinfo_h


#include <QObject>
//#include <QAudioDeviceInfo>
#include <QAudio>
#include <QAudioDevice>
#include <QString>

#define HIFI_AUDIO_DEVICE_INFO_DEFAULT_DEVICE_NAME "default "

class HifiAudioDeviceInfo : public QObject {
    Q_OBJECT
    
public:
    enum DeviceType {
        desktop,
        hmd,
        both
    };

    HifiAudioDeviceInfo() : QObject() {}
    HifiAudioDeviceInfo(const HifiAudioDeviceInfo &deviceInfo) : QObject(){
        _audioDeviceInfo = deviceInfo.getDevice();
        _mode = deviceInfo.getMode();
        _isDefault = deviceInfo.isDefault();
        _deviceType = deviceInfo.getDeviceType();
        _debugName = deviceInfo.getDevice().description();
    }

    HifiAudioDeviceInfo(QAudioDevice deviceInfo, bool isDefault, QAudioDevice::Mode mode, DeviceType devType=both) :
        _audioDeviceInfo(deviceInfo),
        _isDefault(isDefault),
        _mode(mode),
        _deviceType(devType),
        _debugName(deviceInfo.description()) {
    }
    
    void setMode(QAudioDevice::Mode mode) { _mode = mode; }
    void setIsDefault() { _isDefault = true; }
    void setDevice(QAudioDevice devInfo);
    QString deviceName() const {
#if defined(Q_OS_ANDROID)
        return _audioDeviceInfo.deviceName();
#endif
        if (_isDefault) {
            return DEFAULT_DEVICE_NAME;
        } else {
            return _audioDeviceInfo.description();
        }
    }
    QAudioDevice getDevice() const { return _audioDeviceInfo; }
    bool isDefault() const { return _isDefault; }
    QAudioDevice::Mode getMode() const { return _mode; }
    DeviceType getDeviceType() const { return _deviceType; }
    HifiAudioDeviceInfo& operator=(const HifiAudioDeviceInfo& other);
    bool operator==(const HifiAudioDeviceInfo& rhs) const;
    bool operator!=(const HifiAudioDeviceInfo& rhs) const;

private:
    QAudioDevice _audioDeviceInfo;
    bool _isDefault { false };
    QAudioDevice::Mode _mode { QAudioDevice::Input };
    DeviceType _deviceType{ both };
    QString _debugName;

public:
    static const QString DEFAULT_DEVICE_NAME;
};

#endif