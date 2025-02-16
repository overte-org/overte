#define BITFIELD int

#if defined(VULKAN)
    #define gl_InstanceID  gl_InstanceIndex
    #define gl_VertexID  gl_VertexIndex
    #extension GL_ARB_shading_language_420pack : require
    #define UNIFORM_BUFFER(SLOT, NAME) layout(std140, binding=SLOT) uniform NAME
    #define TEXTURE(SLOT, TYPE, NAME) layout(binding=SLOT) uniform TYPE NAME
    #define RESOURCE_BUFFER(SLOT, NAME) layout(binding=SLOT) uniform samplerBuffer NAME
#else
    #define UNIFORM_BUFFER(SLOT, NAME) layout(std140) uniform NAME
    #define TEXTURE(SLOT, TYPE, NAME) uniform TYPE NAME
    #define RESOURCE_BUFFER(SLOT, NAME) uniform samplerBuffer NAME
#endif

#define INPUT(SLOT, TYPE, NAME) layout(location=SLOT) in TYPE NAME
#define OUTPUT(SLOT, TYPE, NAME) layout(location=SLOT) out TYPE NAME
#define FLAT_INPUT(SLOT, TYPE, NAME) layout(location=SLOT) flat in TYPE NAME
#define FLAT_OUTPUT(SLOT, TYPE, NAME) layout(location=SLOT) flat out TYPE NAME
