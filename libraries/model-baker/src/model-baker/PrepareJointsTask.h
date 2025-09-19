//
//  PrepareJointsTask.h
//  model-baker/src/model-baker
//
//  Created by Sabrina Shanman on 2019/01/25.
//  Copyright 2019 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_PrepareJointsTask_h
#define hifi_PrepareJointsTask_h

#include <shared/HifiTypes.h>
#include <hfm/HFM.h>

#include "Engine.h"
#include "BakerTypes.h"

// The property "passthrough", when enabled, will let the input joints flow to the output unmodified, unlike the disabled property, which discards the data
class PrepareJointsConfig : public baker::JobConfig {
    Q_OBJECT
    Q_PROPERTY(bool passthrough MEMBER passthrough)
public:
    bool passthrough { false };
};

class PrepareJointsTask {
public:
    using Config = PrepareJointsConfig;
    using Input = baker::VaryingSet2<std::vector<hfm::Joint>, hifi::VariantMultiHash /*mapping*/>;
    using Output = baker::VaryingSet3<std::vector<hfm::Joint>, QMap<int, glm::quat> /*jointRotationOffsets*/, QHash<QString, int> /*jointIndices*/>;
    using JobModel = baker::Job::ModelIO<PrepareJointsTask, Input, Output, Config>;

    void configure(const Config& config);
    void run(const baker::BakeContextPointer& context, const Input& input, Output& output);

protected:
    bool _passthrough { false };
};

#endif // hifi_PrepareJointsTask_h
