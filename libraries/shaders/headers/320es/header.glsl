#define BITFIELD highp int

#extension GL_EXT_texture_buffer : enable

#if defined(HAVE_EXT_clip_cull_distance) && !defined(VULKAN)
#extension GL_EXT_clip_cull_distance : enable
#endif

#ifdef VULKAN
    #define gl_InstanceID  gl_InstanceIndex
    #define gl_VertexID  gl_VertexIndex
#endif

#define UNIFORM_BUFFER(SLOT, NAME) layout(std140, binding=SLOT) uniform NAME
#define TEXTURE(SLOT, TYPE, NAME) layout(binding=SLOT) uniform TYPE NAME
#define RESOURCE_BUFFER(SLOT, NAME) layout(binding=SLOT) uniform samplerBuffer NAME
#define INPUT(SLOT, TYPE, NAME) layout(location=SLOT) in TYPE NAME
#define OUTPUT(SLOT, TYPE, NAME) layout(location=SLOT) out TYPE NAME
#define FLAT_INPUT(SLOT, TYPE, NAME) layout(location=SLOT) flat in TYPE NAME
#define FLAT_OUTPUT(SLOT, TYPE, NAME) layout(location=SLOT) flat out TYPE NAME

precision highp float;
precision highp samplerBuffer;
precision highp sampler2DShadow;
precision highp sampler2DArrayShadow;
