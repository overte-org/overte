//
//  OpusCodecManager.h
//  plugins/opusCodec/src
//
//  Copyright 2020 Dale Glass
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <PerfStat.h>
#include <QtCore/QLoggingCategory>
#include <opus/opus.h>

#include "OpusEncoder.h"

static QLoggingCategory encoder("AthenaOpusEncoder");

static QString errorToString(int error) {
    switch (error) {
        case OPUS_OK:
            return "OK";
        case OPUS_BAD_ARG:
            return "One or more invalid/out of range arguments.";
        case OPUS_BUFFER_TOO_SMALL:
            return "The mode struct passed is invalid.";
        case OPUS_INTERNAL_ERROR:
            return "An internal error was detected.";
        case OPUS_INVALID_PACKET:
            return "The compressed data passed is corrupted.";
        case OPUS_UNIMPLEMENTED:
            return "Invalid/unsupported request number.";
        case OPUS_INVALID_STATE:
            return "An encoder or decoder structure is invalid or already freed.";
        default:
            return QString("Unknown error code: %i").arg(error);
    }
}



AthenaOpusEncoder::AthenaOpusEncoder(int sampleRate, int numChannels) {
    _opusSampleRate = sampleRate;
    _opusChannels = numChannels;

    int error;

    _encoder = opus_encoder_create(sampleRate, numChannels, DEFAULT_APPLICATION, &error);

    if (error != OPUS_OK) {
        qCCritical(encoder) << "Failed to initialize Opus encoder: " << errorToString(error);
        _encoder = nullptr;
        return;
    }

    setBitrate(DEFAULT_BITRATE);
    setComplexity(DEFAULT_COMPLEXITY);
    setApplication(Encoder::ApplicationType::Audio);
    setSignalType(Encoder::SignalType::Auto);

    qCDebug(encoder) << "Opus encoder initialized, sampleRate = " << sampleRate << "; numChannels = " << numChannels;
}

AthenaOpusEncoder::~AthenaOpusEncoder() {
    opus_encoder_destroy(_encoder);
}



void AthenaOpusEncoder::encode(const QByteArray& decodedBuffer, QByteArray& encodedBuffer) {

    PerformanceTimer perfTimer("AthenaOpusEncoder::encode");
    assert(_encoder);

    encodedBuffer.resize(decodedBuffer.size());
    int frameSize = decodedBuffer.length() / _opusChannels / static_cast<int>(sizeof(opus_int16));

    int bytes = opus_encode(_encoder, reinterpret_cast<const opus_int16*>(decodedBuffer.constData()), frameSize,
        reinterpret_cast<unsigned char*>(encodedBuffer.data()), encodedBuffer.size() );

    if (bytes >= 0) {
        encodedBuffer.resize(bytes);
    } else {
        encodedBuffer.resize(0);

        qCWarning(encoder) << "Error when encoding " << decodedBuffer.length() << " bytes of audio: "
            << errorToString(bytes);
    }

}

int AthenaOpusEncoder::getComplexity() const {
    assert(_encoder);
    int returnValue;
    opus_encoder_ctl(_encoder, OPUS_GET_COMPLEXITY(&returnValue));
    return returnValue*10;
}

void AthenaOpusEncoder::setComplexity(int complexity) {
    assert(_encoder);
    complexity = qBound(0, complexity/10, 10); // The generic interface is 0 to 100, Opus is 0 to 10.

    int returnValue = opus_encoder_ctl(_encoder, OPUS_SET_COMPLEXITY(complexity));

    if (returnValue != OPUS_OK) {
        qCWarning(encoder) << "Error when setting complexity to " << complexity << ": " << errorToString(returnValue);
    }
}

int AthenaOpusEncoder::getBitrate() const {
    assert(_encoder);
    int returnValue;
    opus_encoder_ctl(_encoder, OPUS_GET_BITRATE(&returnValue));
    return returnValue;
}

void AthenaOpusEncoder::setBitrate(int bitrate) {
    assert(_encoder);
    bitrate = qBound(2400, bitrate, 512000); // Opus limits
    int errorCode = opus_encoder_ctl(_encoder, OPUS_SET_BITRATE(bitrate));

    if (errorCode != OPUS_OK) {
        qCWarning(encoder) << "Error when setting bitrate to " << bitrate << ": " << errorToString(errorCode);
    }
}

bool AthenaOpusEncoder::getVBR() const {
    assert(_encoder);
    int returnValue;
    opus_encoder_ctl(_encoder, OPUS_GET_VBR(&returnValue));
    return (bool)returnValue;
}

void AthenaOpusEncoder::setVBR(bool vbr) {
    assert(_encoder);
    int errorCode = opus_encoder_ctl(_encoder, OPUS_SET_VBR(int(vbr)));

    if (errorCode != OPUS_OK) {
        qCWarning(encoder) << "Error when setting VBR to " << vbr << ": " << errorToString(errorCode);
    }
}

int AthenaOpusEncoder::getVBRConstraint() const {
    assert(_encoder);
    int returnValue;
    opus_encoder_ctl(_encoder, OPUS_GET_VBR_CONSTRAINT(&returnValue));
    return returnValue;
}

void AthenaOpusEncoder::setVBRConstraint(int vbr_const) {
    assert(_encoder);
    int errorCode = opus_encoder_ctl(_encoder, OPUS_SET_VBR_CONSTRAINT(vbr_const));

    if (errorCode != OPUS_OK) {
        qCWarning(encoder) << "Error when setting VBR constraint to " << vbr_const << ": " << errorToString(errorCode);
    }
}

int AthenaOpusEncoder::getMaxBandwidth() const {
    assert(_encoder);
    int returnValue;
    opus_encoder_ctl(_encoder, OPUS_GET_MAX_BANDWIDTH(&returnValue));
    return returnValue;
}

void AthenaOpusEncoder::setMaxBandwidth(int maxBandwidth) {
    assert(_encoder);
    int errorCode = opus_encoder_ctl(_encoder, OPUS_SET_MAX_BANDWIDTH(maxBandwidth));

    if (errorCode != OPUS_OK) {
        qCWarning(encoder) << "Error when setting max bandwidth to " << maxBandwidth << ": " << errorToString(errorCode);
    }
}

int AthenaOpusEncoder::getBandwidth() const {
    assert(_encoder);
    int bandwidth;
    opus_encoder_ctl(_encoder, OPUS_GET_BANDWIDTH(&bandwidth));
    return bandwidth;
}

void AthenaOpusEncoder::setBandwidth(int bandwidth) {
    assert(_encoder);
    int errorCode = opus_encoder_ctl(_encoder, OPUS_SET_BANDWIDTH(bandwidth));

    if (errorCode != OPUS_OK) {
        qCWarning(encoder) << "Error when setting bandwidth to " << bandwidth << ": " << errorToString(errorCode);
    }
}

Encoder::SignalType AthenaOpusEncoder::getSignalType() const {
    assert(_encoder);
    int signal;
    opus_encoder_ctl(_encoder, OPUS_GET_SIGNAL(&signal));

    switch(signal) {
        case OPUS_AUTO:
            return SignalType::Auto;
        case OPUS_SIGNAL_MUSIC:
            return SignalType::Music;
        case OPUS_SIGNAL_VOICE:
            return SignalType::Voice;
        default:
            qCCritical(encoder) << "Opus returned unrecognized signal type" << signal;
            return SignalType::Auto;
    }

}

void AthenaOpusEncoder::setSignalType(Encoder::SignalType signal) {
    assert(_encoder);
    int sig = OPUS_SIGNAL_MUSIC;

    switch(signal) {
        case SignalType::Auto:
            sig = OPUS_AUTO;
            break;
        case SignalType::Music:
            sig = OPUS_SIGNAL_MUSIC;
            break;
        case SignalType::Voice:
            sig = OPUS_SIGNAL_VOICE;
            break;
    }

    int errorCode = opus_encoder_ctl(_encoder, OPUS_SET_SIGNAL(sig));

    if (errorCode != OPUS_OK) {
        qCWarning(encoder) << "Error when setting signal to " << sig << ": " << errorToString(errorCode);
    }
}

Encoder::ApplicationType AthenaOpusEncoder::getApplication() const {
    assert(_encoder);
    int applicationValue;
    opus_encoder_ctl(_encoder, OPUS_GET_APPLICATION(&applicationValue));

    switch(applicationValue) {
        case OPUS_APPLICATION_AUDIO:
            return ApplicationType::Audio;
        case OPUS_APPLICATION_RESTRICTED_LOWDELAY:
            return ApplicationType::LowDelay;
        case OPUS_APPLICATION_VOIP:
            return ApplicationType::VOIP;
        default:
            qCCritical(encoder) << "Opus returned unrecognized application value" << applicationValue;
            return ApplicationType::Audio;
    }
}

void AthenaOpusEncoder::setApplication(Encoder::ApplicationType application) {
    assert(_encoder);
    int app = OPUS_APPLICATION_AUDIO;

    switch(application) {
        case ApplicationType::Audio:
            app = OPUS_APPLICATION_AUDIO;
            break;
        case ApplicationType::LowDelay:
            app = OPUS_APPLICATION_RESTRICTED_LOWDELAY;
            break;
        case ApplicationType::VOIP:
            app = OPUS_APPLICATION_VOIP;
            break;
    }

    int errorCode = opus_encoder_ctl(_encoder, OPUS_SET_APPLICATION(app));

    if (errorCode != OPUS_OK) {
        qCWarning(encoder) << "Error when setting application to " << app << ": " << errorToString(errorCode);
    }
}

int AthenaOpusEncoder::getLookahead() const {
    assert(_encoder);
    int lookAhead;
    opus_encoder_ctl(_encoder, OPUS_GET_LOOKAHEAD(&lookAhead));
    return lookAhead;
}

bool AthenaOpusEncoder::getFEC() const {
    assert(_encoder);
    int fec;
    opus_encoder_ctl(_encoder, OPUS_GET_INBAND_FEC(&fec));
    return (bool)fec;
}

void AthenaOpusEncoder::setFEC(bool inBandFEC) {
    assert(_encoder);
    int errorCode = opus_encoder_ctl(_encoder, OPUS_SET_INBAND_FEC((int)inBandFEC));

    if (errorCode != OPUS_OK) {
        qCWarning(encoder) << "Error when setting inband FEC to " << inBandFEC << ": " << errorToString(errorCode);
    }
}

int AthenaOpusEncoder::getPacketLossPercent() const {
    assert(_encoder);
    int lossPercentage;
    opus_encoder_ctl(_encoder, OPUS_GET_PACKET_LOSS_PERC(&lossPercentage));
    return lossPercentage;
}

void AthenaOpusEncoder::setPacketLossPercent(int percentage) {
    assert(_encoder);
    int errorCode = opus_encoder_ctl(_encoder, OPUS_SET_PACKET_LOSS_PERC(percentage));

    if (errorCode != OPUS_OK) {
        qCWarning(encoder) << "Error when setting loss percent to " << percentage << ": " << errorToString(errorCode);
    }
}

int AthenaOpusEncoder::getDTX() const {
    assert(_encoder);
    int dtx;
    opus_encoder_ctl(_encoder, OPUS_GET_DTX(&dtx));
    return dtx;
}

void AthenaOpusEncoder::setDTX(int dtx) {
    assert(_encoder);
    int errorCode = opus_encoder_ctl(_encoder, OPUS_SET_DTX(dtx));

    if (errorCode != OPUS_OK) {
        qCWarning(encoder) << "Error when setting DTX to " << dtx << ": " << errorToString(errorCode);
    }
}
