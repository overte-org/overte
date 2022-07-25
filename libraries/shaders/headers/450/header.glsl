#define GPU_SSBO_TRANSFORM_OBJECT
#define BITFIELD int
#define LAYOUT(X) layout(X)
#define LAYOUT_STD140(X) layout(std140, X)
#ifdef VULKAN
    #define gl_InstanceID  gl_InstanceIndex
    #define gl_VertexID  gl_VertexIndex
    #define UNIFORM_BUFFER(SLOT, NAME) layout(std140, set=0, binding=SLOT) uniform NAME
    #define TEXTURE(SLOT, TYPE, NAME) layout(set=1, binding=SLOT) uniform TYPE NAME
    #define RESOURCE_BUFFER(SLOT, NAME) layout(set=2, binding=SLOT) buffer NAME
#else
    #define UNIFORM_BUFFER(SLOT, NAME) layout(std140, binding=SLOT) uniform NAME
    #define TEXTURE(SLOT, TYPE, NAME) layout(binding=SLOT) uniform TYPE NAME
    #define RESOURCE_BUFFER(SLOT, NAME) layout(binding=SLOT) buffer NAME
#endif

#define INPUT(SLOT, TYPE, NAME) layout(location=SLOT) in TYPE NAME
#define OUTPUT(SLOT, TYPE, NAME) layout(location=SLOT) out TYPE NAME
#define FLAT_INPUT(SLOT, TYPE, NAME) layout(location=SLOT) flat in TYPE NAME
#define FLAT_OUTPUT(SLOT, TYPE, NAME) layout(location=SLOT) flat out TYPE NAME
