//
//  HazeStage.h

//  Created by Nissim Hadar on 9/26/2017.
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_utils_HazeStage_h
#define hifi_render_utils_HazeStage_h

#include <graphics/Haze.h>
#include <render/Stage.h>
#include <render/StageSetup.h>

// Haze stage to set up haze-related rendering tasks
class HazeStage : public render::PointerStage<graphics::Haze, graphics::HazePointer> {};
using HazeStagePointer = std::shared_ptr<HazeStage>;

class HazeStageSetup : public render::StageSetup<HazeStage> {
public:
    using JobModel = render::Job::Model<HazeStageSetup>;
};

class FetchHazeConfig : public render::Job::Config {
    Q_OBJECT

    Q_PROPERTY(glm::vec<3,float,glm::packed_highp> hazeColor MEMBER hazeColor WRITE setHazeColor NOTIFY dirty);
    Q_PROPERTY(float hazeGlareAngle MEMBER hazeGlareAngle WRITE setHazeGlareAngle NOTIFY dirty);

    Q_PROPERTY(glm::vec<3,float,glm::packed_highp> hazeGlareColor MEMBER hazeGlareColor WRITE setHazeGlareColor NOTIFY dirty);
    Q_PROPERTY(float hazeBaseReference MEMBER hazeBaseReference WRITE setHazeBaseReference NOTIFY dirty);

    Q_PROPERTY(bool isHazeActive MEMBER isHazeActive WRITE setHazeActive NOTIFY dirty);
    Q_PROPERTY(bool isAltitudeBased MEMBER isAltitudeBased WRITE setAltitudeBased NOTIFY dirty);
    Q_PROPERTY(bool isHazeAttenuateKeyLight MEMBER isHazeAttenuateKeyLight WRITE setHazeAttenuateKeyLight NOTIFY dirty);
    Q_PROPERTY(bool isModulateColorActive MEMBER isModulateColorActive WRITE setModulateColorActive NOTIFY dirty);
    Q_PROPERTY(bool isHazeEnableGlare MEMBER isHazeEnableGlare WRITE setHazeEnableGlare NOTIFY dirty);

    Q_PROPERTY(float hazeRange MEMBER hazeRange WRITE setHazeRange NOTIFY dirty);
    Q_PROPERTY(float hazeHeight MEMBER hazeHeight WRITE setHazeAltitude NOTIFY dirty);

    Q_PROPERTY(float hazeKeyLightRange MEMBER hazeKeyLightRange WRITE setHazeKeyLightRange NOTIFY dirty);
    Q_PROPERTY(float hazeKeyLightAltitude MEMBER hazeKeyLightAltitude WRITE setHazeKeyLightAltitude NOTIFY dirty);

    Q_PROPERTY(float hazeBackgroundBlend MEMBER hazeBackgroundBlend WRITE setHazeBackgroundBlend NOTIFY dirty);

public:
    FetchHazeConfig() : render::Job::Config() {}

    glm::vec3 hazeColor{ graphics::Haze::INITIAL_HAZE_COLOR };
    float hazeGlareAngle{ graphics::Haze::INITIAL_HAZE_GLARE_ANGLE };

    glm::vec3 hazeGlareColor{ graphics::Haze::INITIAL_HAZE_GLARE_COLOR };
    float hazeBaseReference{ graphics::Haze::INITIAL_HAZE_BASE_REFERENCE };

    bool isHazeActive{ false };
    bool isAltitudeBased{ false };
    bool isHazeAttenuateKeyLight{ false };
    bool isModulateColorActive{ false };
    bool isHazeEnableGlare{ false };

    float hazeRange{ graphics::Haze::INITIAL_HAZE_RANGE };
    float hazeHeight{ graphics::Haze::INITIAL_HAZE_HEIGHT };

    float hazeKeyLightRange{ graphics::Haze::INITIAL_KEY_LIGHT_RANGE };
    float hazeKeyLightAltitude{ graphics::Haze::INITIAL_KEY_LIGHT_ALTITUDE };

    float hazeBackgroundBlend{ graphics::Haze::INITIAL_HAZE_BACKGROUND_BLEND };

public slots:
    void setHazeColor(const glm::vec3 value) { hazeColor = value; emit dirty(); }
    void setHazeGlareAngle(const float value) { hazeGlareAngle = value; emit dirty(); }

    void setHazeGlareColor(const glm::vec3 value) { hazeGlareColor = value; emit dirty(); }
    void setHazeBaseReference(const float value) { hazeBaseReference = value; ; emit dirty(); }

    void setHazeActive(const bool active) { isHazeActive = active; emit dirty(); }
    void setAltitudeBased(const bool active) { isAltitudeBased = active; emit dirty(); }
    void setHazeAttenuateKeyLight(const bool active) { isHazeAttenuateKeyLight = active; emit dirty(); }
    void setModulateColorActive(const bool active) { isModulateColorActive = active; emit dirty(); }
    void setHazeEnableGlare(const bool active) { isHazeEnableGlare = active; emit dirty(); }

    void setHazeRange(const float value) { hazeRange = value; emit dirty(); }
    void setHazeAltitude(const float value) { hazeHeight = value; emit dirty(); }

    void setHazeKeyLightRange(const float value) { hazeKeyLightRange = value; emit dirty(); }
    void setHazeKeyLightAltitude(const float value) { hazeKeyLightAltitude = value; emit dirty(); }

    void setHazeBackgroundBlend(const float value) { hazeBackgroundBlend = value; ; emit dirty(); }

signals:
    void dirty();
};
#endif
