#define BITFIELD int
#define LAYOUT(X) layout(X)
#define LAYOUT_STD140(X) layout(std140, X)
#ifdef VULKAN
#define gl_InstanceID  gl_InstanceIndex
#define gl_VertexID  gl_VertexIndex
    #define UNIFORM_BUFFER(SLOT, NAME) layout(std140, set=0, binding=SLOT) uniform NAME
    #define TEXTURE(SLOT, TYPE, NAME) layout(set=1, binding=SLOT) uniform TYPE NAME
    #define RESOURCE_BUFFER(SLOT, NAME) layout(set=2, binding=SLOT) readonly buffer NAME
    #define RESOURCE_BUFFER_STD140(SLOT, NAME) layout(std140, set=2, binding=SLOT) readonly buffer NAME
#else
    #define UNIFORM_BUFFER(SLOT, NAME) layout(std140, binding=SLOT) uniform NAME
    #define TEXTURE(SLOT, TYPE, NAME) layout(binding=SLOT) uniform TYPE NAME
    #define RESOURCE_BUFFER(SLOT, NAME) layout(binding=SLOT) buffer NAME
    #define RESOURCE_BUFFER_STD140(SLOT, NAME) layout(std140, binding=SLOT) buffer NAME
#endif
#extension GL_EXT_texture_buffer : enable
#if defined(HAVE_EXT_clip_cull_distance) && !defined(VULKAN)
#extension GL_EXT_clip_cull_distance : enable
#endif
precision highp int;
precision highp float;
precision highp samplerBuffer;
precision highp sampler2DShadow;
precision highp sampler2DArrayShadow;
precision highp sampler2DArray;

#define INPUT(SLOT, TYPE, NAME) layout(location=SLOT) in TYPE NAME
#define OUTPUT(SLOT, TYPE, NAME) layout(location=SLOT) out TYPE NAME
#define FLAT_INPUT(SLOT, TYPE, NAME) layout(location=SLOT) flat in TYPE NAME
#define FLAT_OUTPUT(SLOT, TYPE, NAME) layout(location=SLOT) flat out TYPE NAME
