//
//  Created by Bradley Austin Davis on 2017/04/27
//  Copyright 2013-2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_MyHead_h
#define hifi_MyHead_h

#include <avatars-renderer/Head.h>

class MyAvatar;
class MyHead : public Head {
    using Parent = Head;
public:
    explicit MyHead(MyAvatar* owningAvatar);

    /// \return orientationBody * orientationBasePitch
    glm::quat getHeadOrientation() const;
    void simulate(float deltaTime) override;

private:
    Q_DISABLE_COPY(MyHead)
};

#endif // hifi_MyHead_h
