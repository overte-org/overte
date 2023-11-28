// <!
//  Created by Bradley Austin Davis on 2018/05/25
//  Copyright 2013-2018 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
// !>

// <@if not PROCEDURAL_SHADER_CONSTANTS_H@>
// <@def PROCEDURAL_SHADER_CONSTANTS_H@>

// Hack comment to absorb the extra '//' scribe prepends

#ifndef PROCEDURAL_SHADER_CONSTANTS_H
#define PROCEDURAL_SHADER_CONSTANTS_H

#define PROCEDURAL_BUFFER_INPUTS 7

#define PROCEDURAL_UNIFORM_CUSTOM 220

#define PROCEDURAL_TEXTURE_CHANNEL0 2
#define PROCEDURAL_TEXTURE_CHANNEL1 3
#define PROCEDURAL_TEXTURE_CHANNEL2 4
#define PROCEDURAL_TEXTURE_CHANNEL3 5

#define PROCEDURAL_PARTICLE_TEXTURE_PROP0 6
#define PROCEDURAL_PARTICLE_TEXTURE_PROP1 7
#define PROCEDURAL_PARTICLE_TEXTURE_PROP2 8
#define PROCEDURAL_PARTICLE_TEXTURE_PROP3 9
#define PROCEDURAL_PARTICLE_TEXTURE_PROP4 10

// <!

namespace procedural { namespace slot {

namespace buffer {
enum Bufffer {
    Inputs = PROCEDURAL_BUFFER_INPUTS,
};
}

namespace uniform {
enum Uniform {
    Custom = PROCEDURAL_UNIFORM_CUSTOM,
};
}

namespace texture {
enum Texture {
    Channel0 = PROCEDURAL_TEXTURE_CHANNEL0,
    Channel1 = PROCEDURAL_TEXTURE_CHANNEL1,
    Channel2 = PROCEDURAL_TEXTURE_CHANNEL2,
    Channel3 = PROCEDURAL_TEXTURE_CHANNEL3,

    ParticleProp0 = PROCEDURAL_PARTICLE_TEXTURE_PROP0,
    ParticleProp1 = PROCEDURAL_PARTICLE_TEXTURE_PROP1,
    ParticleProp2 = PROCEDURAL_PARTICLE_TEXTURE_PROP2,
    ParticleProp3 = PROCEDURAL_PARTICLE_TEXTURE_PROP3,
    ParticleProp4 = PROCEDURAL_PARTICLE_TEXTURE_PROP4,
};
} // namespace texture

} } // namespace procedural::slot

// !>
// Hack Comment

#endif // PROCEDURAL_SHADER_CONSTANTS_H

// <@if 1@>
// Trigger Scribe include
// <@endif@> <!def that !>

// <@endif@>

// Hack Comment
