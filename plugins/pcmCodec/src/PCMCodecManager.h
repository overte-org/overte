//
//  PCMCodecManager.h
//  plugins/pcmCodec/src
//
//  Created by Brad Hefta-Gaub on 6/9/2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi__PCMCodecManager_h
#define hifi__PCMCodecManager_h

#include <plugins/CodecPlugin.h>
#include <AudioConstants.h>

class PCMCodec : public CodecPlugin, public Encoder, public Decoder {
    Q_OBJECT

public:
    // Plugin functions
    bool isSupported() const override;
    const QString getName() const override { return NAME; }

    void init() override;
    void deinit() override;

    /// Called when a plugin is being activated for use.  May be called multiple times.
    bool activate() override;
    /// Called when a plugin is no longer being used.  May be called multiple times.
    void deactivate() override;

    bool isLossless() const override { return true; }

    virtual Encoder* createEncoder(int sampleRate, int numChannels) override;
    virtual Decoder* createDecoder(int sampleRate, int numChannels) override;
    virtual void releaseEncoder(Encoder* encoder) override;
    virtual void releaseDecoder(Decoder* decoder) override;

    virtual void encode(const QByteArray& decodedBuffer, QByteArray& encodedBuffer) override {
        encodedBuffer = decodedBuffer;
    }

    virtual void decode(const QByteArray& encodedBuffer, QByteArray& decodedBuffer) override {
        decodedBuffer = encodedBuffer;
    }

    virtual void lostFrame(QByteArray& decodedBuffer) override {
        decodedBuffer.resize(AudioConstants::NETWORK_FRAME_BYTES_STEREO);
        memset(decodedBuffer.data(), 0, decodedBuffer.size());
    }

private:
    static const char* NAME;
};

class zLibCodec : public CodecPlugin, public Encoder, public Decoder {
    Q_OBJECT

public:
    // Plugin functions
    bool isSupported() const override;
    const QString getName() const override { return NAME; }

    bool isLossless() const override { return true; }

    bool hasComplexity() const override { return true; }

    void setComplexity(int complexity) override { _compressionLevel = qBound(0, complexity/11, 9); }

    void init() override;
    void deinit() override;

    /// Called when a plugin is being activated for use.  May be called multiple times.
    bool activate() override;
    /// Called when a plugin is no longer being used.  May be called multiple times.
    void deactivate() override;

    virtual Encoder* createEncoder(int sampleRate, int numChannels) override;
    virtual Decoder* createDecoder(int sampleRate, int numChannels) override;
    virtual void releaseEncoder(Encoder* encoder) override;
    virtual void releaseDecoder(Decoder* decoder) override;

    virtual void encode(const QByteArray& decodedBuffer, QByteArray& encodedBuffer) override {
        encodedBuffer = qCompress(decodedBuffer, _compressionLevel);
    }

    virtual void decode(const QByteArray& encodedBuffer, QByteArray& decodedBuffer) override {
        decodedBuffer = qUncompress(encodedBuffer);
    }

    virtual void lostFrame(QByteArray& decodedBuffer) override {
        decodedBuffer.resize(AudioConstants::NETWORK_FRAME_BYTES_STEREO);
        memset(decodedBuffer.data(), 0, decodedBuffer.size());
    }

private:
    static const char* NAME;
    int _compressionLevel = 6; // Default compression level for zLib
};

#endif // hifi__PCMCodecManager_h
