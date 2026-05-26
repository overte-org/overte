//
//  State
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 3/8/2015.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_State_h
#define hifi_gpu_State_h

#include "Format.h"

#include <memory>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <bitset>
#include <QString>

#include "Sampler.h"

// Why a macro and not a fancy template you will ask me ?
// Because some of the fields are bool packed tightly in the State::Cache class
// and it s just not good anymore for template T& variable manipulation...
#define SET_FIELD(FIELD, PATH, value) \
    {                                 \
        _values.PATH = value;         \
        if (value == DEFAULT.PATH) {  \
            _signature.reset(FIELD);  \
        } else {                      \
            _signature.set(FIELD);    \
        }                             \
        _stamp++;                     \
    }

namespace gpu {

class GPUObject;

/// Describes pipeline state.
class State {
public:
    State();
    State(const State& state) : _values(state._values), _signature(state._signature), _stamp(state._stamp) {}
    virtual ~State();

    /**
     * @brief Get the current stamp.
     *
     * Stamp is used to detect changes in the pipeline state.
     * @return Current stamp.
     */
    Stamp getStamp() const { return _stamp; }

    enum FillMode
    {
        FILL_POINT = 0,
        FILL_LINE,
        FILL_FACE,

        NUM_FILL_MODES,
    };

    enum CullMode
    {
        CULL_NONE = 0,
        CULL_FRONT,
        CULL_BACK,

        NUM_CULL_MODES,
    };

    enum StencilOp
    {
        STENCIL_OP_KEEP = 0,
        STENCIL_OP_ZERO,
        STENCIL_OP_REPLACE,
        STENCIL_OP_INCR_SAT,
        STENCIL_OP_DECR_SAT,
        STENCIL_OP_INVERT,
        STENCIL_OP_INCR,
        STENCIL_OP_DECR,

        NUM_STENCIL_OPS,
    };

    enum BlendArg
    {
        ZERO = 0,
        ONE,
        SRC_COLOR,
        INV_SRC_COLOR,
        SRC_ALPHA,
        INV_SRC_ALPHA,
        DEST_ALPHA,
        INV_DEST_ALPHA,
        DEST_COLOR,
        INV_DEST_COLOR,
        SRC_ALPHA_SAT,
        FACTOR_COLOR,
        INV_FACTOR_COLOR,
        FACTOR_ALPHA,
        INV_FACTOR_ALPHA,

        NUM_BLEND_ARGS,
    };

    enum BlendOp
    {
        BLEND_OP_ADD = 0,
        BLEND_OP_SUBTRACT,
        BLEND_OP_REV_SUBTRACT,
        BLEND_OP_MIN,
        BLEND_OP_MAX,

        NUM_BLEND_OPS,
    };

    enum ColorMask
    {
        WRITE_NONE = 0,
        WRITE_RED = 1,
        WRITE_GREEN = 2,
        WRITE_BLUE = 4,
        WRITE_ALPHA = 8,
        WRITE_ALL = (WRITE_RED | WRITE_GREEN | WRITE_BLUE | WRITE_ALPHA),
    };

    /// Represents setting for depth testing and writing.
    class DepthTest {
    public:
        /// Enable writing to the depth buffer.
        uint8 writeMask{ true };

        /// Enable depth test.
        uint8 enabled{ false };

        ComparisonFunction function{ ComparisonFunction::LESS };

    public:
        /**
         *
         * @param enabled `true` if depth testing is enabled.
         * @param writeMask `true` if writing to depth buffer is enabled.
         * @param func Comparison function to use for depth thest.
         */
        DepthTest(bool enabled = false, bool writeMask = true, ComparisonFunction func = ComparisonFunction::LESS) :
            writeMask(writeMask), enabled(enabled), function(func) {}

        /**
         * @return `true` if depth testing is enabled.
         */
        bool isEnabled() const { return enabled != 0; }

        /**
         * @return Comparison function used for depth testing.
         */
        ComparisonFunction getFunction() const { return function; }

        /**
         * @return `true` if writing to depth buffer is enabled.
         */
        uint8 getWriteMask() const { return writeMask; }

        bool operator==(const DepthTest& right) const {
            return
                    writeMask == right.writeMask &&
                    enabled == right.enabled &&
                    function == right.function;
        }

        bool operator!=(const DepthTest& right) const {
            return !(*this == right);
        }

        operator QString() const {
            return QString("{ writeMask = %1, enabled = %2, function = %3 }").arg(writeMask).arg(enabled).arg((uint8_t)function);
        }
    };

    ///
    struct StencilTest {
        ComparisonFunction function;
        StencilOp failOp;
        StencilOp depthFailOp;
        StencilOp passOp;
        int8 reference{ 0 };
        uint8 readMask{ 0xff };

    public:
        /**
         * @param reference Value to which stored stencil value is compared.
         * @param readMask Value that is bit AND with both reference and stored value before comparison.
         * @param func Comparison function.
         * @param failOp Operation performed when comparison function is false.
         * @param depthFailOp Operation performed when stencil test passes but depth thest fails.
         * @param passOp Operation performed when stencil test passes and depth thest passes or is disabled.
         */
        StencilTest(int8 reference = 0,
                    uint8 readMask = 0xFF,
                    ComparisonFunction func = ComparisonFunction::ALWAYS,
                    StencilOp failOp = STENCIL_OP_KEEP,
                    StencilOp depthFailOp = STENCIL_OP_KEEP,
                    StencilOp passOp = STENCIL_OP_KEEP) :
            function(func),
            failOp(failOp), depthFailOp(depthFailOp), passOp(passOp), reference(reference), readMask(readMask) {}

        /**
         * @return Comparison function that is performed between reference value and stored stencil value.
         */
        ComparisonFunction getFunction() const { return function; }

        /**
         * @return Operation performed when comparison function is false.
         */
        StencilOp getFailOp() const { return failOp; }

        /**
         * @return Operation performed when stencil test passes but depth thest fails.
         */
        StencilOp getDepthFailOp() const { return depthFailOp; }

        /**
         * @return Operation performed when stencil test passes and depth thest passes or is disabled.
         */
        StencilOp getPassOp() const { return passOp; }

        /**
         * @return Value to which stored stencil value is compared.
         */
        int8 getReference() const { return reference; }

        /**
         * @return Value that is bit AND with both reference and stored value before comparison.
         */
        uint8 getReadMask() const { return readMask; }

        bool operator==(const StencilTest& right) const {
            return
                    function == right.function &&
                    failOp == right.failOp &&
                    depthFailOp == right.depthFailOp &&
                    passOp == right.passOp &&
                    reference == right.reference &&
                    readMask == right.readMask;

        }

        bool operator!=(const StencilTest &right) const { return !(right==*this); }
    };

    ///
    struct StencilActivation {
        uint8 frontWriteMask = 0xFF;
        uint8 backWriteMask = 0xFF;
        bool enabled;

    public:
        /**
         * @param enabled `true` enables stencil test.
         * @param frontWriteMask Controls which bits are written to in the stencil buffer for front faces.
         * @param backWriteMask Controls which bits are written to in the stencil buffer for back faces.
         */
        StencilActivation(bool enabled = false, uint8 frontWriteMask = 0xFF, uint8 backWriteMask = 0xFF) :
            frontWriteMask(frontWriteMask), backWriteMask(backWriteMask), enabled(enabled) {}

        /**
         * @return `true` if stencil test is enabled, `false` if it's disabled.
         */
        bool isEnabled() const { return enabled; }

        /**
         * @return Bit mask controlling which bits are written to in the stencil buffer for front faces.
         */
        uint8 getWriteMaskFront() const { return frontWriteMask; }

        /**
         * @return Bit mask controlling which bits are written to in the stencil buffer for back faces.
         */
        uint8 getWriteMaskBack() const { return backWriteMask; }

        bool operator==(const StencilActivation& right) const {
            return
                frontWriteMask == right.frontWriteMask &&
                backWriteMask == right.backWriteMask &&
                enabled == right.enabled;
        }

        bool operator!=(const StencilActivation& right) const {
            return !(*this == right);
        }
    };

    /// Represents blending state.
    struct BlendFunction {
        // Using uint8 here will make the structure as a whole not align to 32 bits
        /// `true` if blending is enabled.
        uint16 enabled;

        /// Determines how source color will be used.
        BlendArg sourceColor;

        /// Determines how source alpha will be used.
        BlendArg sourceAlpha;

        /// Determines how destination color will be used.
        BlendArg destColor;

        /// Determines how destination alpha will be used.
        BlendArg destAlpha;

        /// Type of operation that will be performed on color if blending is enabled.
        BlendOp opColor;

        /// Type of operation that will be performed on alpha if blending is enabled.
        BlendOp opAlpha;

    public:
        /**
         *
         * @param enabled `true` if blending is enabled.
         * @param sourceColor Determines how source color will be used.
         * @param operationColor Type of operation that will be performed on color if blending is enabled.
         * @param destinationColor Determines how destination color will be used.
         * @param sourceAlpha Determines how source alpha will be used.
         * @param operationAlpha Type of operation that will be performed on alpha if blending is enabled.
         * @param destinationAlpha Determines how destination alpha will be used.
         */
        BlendFunction(bool enabled,
                      BlendArg sourceColor,
                      BlendOp operationColor,
                      BlendArg destinationColor,
                      BlendArg sourceAlpha,
                      BlendOp operationAlpha,
                      BlendArg destinationAlpha) :
            enabled(enabled),
            sourceColor(sourceColor), sourceAlpha(sourceAlpha),
            destColor(destinationColor), destAlpha(destinationAlpha),
            opColor(operationColor), opAlpha(operationAlpha) {}

        /**
         * @param enabled `true` if blending is enabled.
         * @param source Determines how source (both color and alpha) will be used.
         * @param operation Type of operation that will be performed on both color and alpha if blending is enabled.
         * @param destination Determines how destination (both color and alpha) will be used.
         */
        BlendFunction(bool enabled = false, BlendArg source = ONE, BlendOp operation = BLEND_OP_ADD, BlendArg destination = ZERO) :
            BlendFunction(enabled, source, operation, destination, source, operation, destination) {}

        /**
         * @return `true` if blending is enabled.
         */
        bool isEnabled() const { return (enabled != 0); }

        /**
         * @return Enum determining how source color will be used.
         */
        BlendArg getSourceColor() const { return sourceColor; }

        /**
         * @return Enum determining how destination color will be used.
         */
        BlendArg getDestinationColor() const { return destColor; }

        /**
         * @return Type of operation that will be performed if blending is enabled.
         */
        BlendOp getOperationColor() const { return opColor; }

        /**
         * @return Enum determining how source alpha will be used.
         */
        BlendArg getSourceAlpha() const { return sourceAlpha; }

        /**
         * @return Enum determining how destination alpha will be used.
         */
        BlendArg getDestinationAlpha() const { return destAlpha; }

        /**
         * @return Type of operation that will be performed on alpha if blending is enabled.
         */
        BlendOp getOperationAlpha() const { return opAlpha; }

        bool operator==(const BlendFunction& right) const {
            return
                    enabled == right.enabled &&
                    sourceColor == right.sourceColor &&
                    sourceAlpha == right.sourceAlpha &&
                    destColor == right.destColor &&
                    destAlpha == right.destAlpha &&
                    opColor == right.opColor &&
                    opAlpha == right.opAlpha;

        }

        bool operator!=(const BlendFunction& right) const {
            return !(*this == right);
        }
    };

    /// Flags for enabling or disabling various settings for the pipeline.
    struct Flags {
        Flags() :
            frontFaceClockwise(false), depthClampEnable(false), scissorEnable(false), multisampleEnable(false),
            antialisedLineEnable(true), alphaToCoverageEnable(false) {}
        bool frontFaceClockwise;
        bool depthClampEnable;
        bool scissorEnable;
        bool multisampleEnable;
        bool antialisedLineEnable;
        bool alphaToCoverageEnable;


        bool operator==(const Flags& right) const {
            return
                    frontFaceClockwise == right.frontFaceClockwise &&
                    depthClampEnable == right.depthClampEnable &&
                    scissorEnable == right.scissorEnable &&
                    multisampleEnable == right.multisampleEnable &&
                    antialisedLineEnable == right.antialisedLineEnable &&
                    alphaToCoverageEnable == right.alphaToCoverageEnable;

        }

        bool operator!=(const Flags& right) const {
            return !(*this == right);
        }
    };

    /**
     * @brief The Data class is the full explicit description of the State class fields value.
     *
     * Useful for having one const static called Default for reference or for the gpu::Backend to keep track of the current value.
     */
    class Data {
    public:
        float depthBias = 0.0f;
        float depthBiasSlopeScale = 0.0f;

        DepthTest depthTest;
        StencilActivation stencilActivation;
        StencilTest stencilTestFront;
        StencilTest stencilTestBack;
        uint32 sampleMask = 0xFFFFFFFF;
        BlendFunction blendFunction;
        FillMode fillMode{ FILL_FACE };
        CullMode cullMode{ CULL_NONE };
        ColorMask colorWriteMask{ WRITE_ALL };

        Flags flags;
    };

    /**
     * @return Returns a text string that is unique to a given pipeline state setup.
     */
    std::string getKey() const;

    /// The unique default values for all the fields
    static const Data DEFAULT;
    void setFillMode(FillMode fill) { SET_FIELD(FILL_MODE, fillMode, fill); }
    FillMode getFillMode() const { return _values.fillMode; }

    void setCullMode(CullMode cull) { SET_FIELD(CULL_MODE, cullMode, cull); }
    CullMode getCullMode() const { return _values.cullMode; }

    /**
     * @return Object containing settings enabling or disabling various parts of the pipeline.
     */
    const Flags& getFlags() const { return _values.flags; }

    void setFrontFaceClockwise(bool isClockwise) { SET_FIELD(FRONT_FACE_CLOCKWISE, flags.frontFaceClockwise, isClockwise); }
    bool isFrontFaceClockwise() const { return _values.flags.frontFaceClockwise; }

    void setDepthClampEnable(bool enable) { SET_FIELD(DEPTH_CLAMP_ENABLE, flags.depthClampEnable, enable); }
    bool isDepthClampEnable() const { return _values.flags.depthClampEnable; }

    void setScissorEnable(bool enable) { SET_FIELD(SCISSOR_ENABLE, flags.scissorEnable, enable); }
    bool isScissorEnable() const { return _values.flags.scissorEnable; }

    void setMultisampleEnable(bool enable) { SET_FIELD(MULTISAMPLE_ENABLE, flags.multisampleEnable, enable); }
    bool isMultisampleEnable() const { return _values.flags.multisampleEnable; }

    void setAntialiasedLineEnable(bool enable) { SET_FIELD(ANTIALISED_LINE_ENABLE, flags.antialisedLineEnable, enable); }
    bool isAntialiasedLineEnable() const { return _values.flags.antialisedLineEnable; }

    // Depth Bias
    void setDepthBias(float bias) { SET_FIELD(DEPTH_BIAS, depthBias, bias); }
    float getDepthBias() const { return _values.depthBias; }

    void setDepthBiasSlopeScale(float scale) { SET_FIELD(DEPTH_BIAS_SLOPE_SCALE, depthBiasSlopeScale, scale); }
    float getDepthBiasSlopeScale() const { return _values.depthBiasSlopeScale; }

    // Depth Test
    void setDepthTest(DepthTest newDepthTest) { SET_FIELD(DEPTH_TEST, depthTest, newDepthTest); }
    void setDepthTest(bool enable, bool writeMask, ComparisonFunction func) {
        setDepthTest(DepthTest(enable, writeMask, func));
    }
    DepthTest getDepthTest() const { return _values.depthTest; }

    bool isDepthTestEnabled() const { return getDepthTest().isEnabled(); }
    uint8 getDepthTestWriteMask() const { return getDepthTest().getWriteMask(); }
    ComparisonFunction getDepthTestFunc() const { return getDepthTest().getFunction(); }

    // Stencil test
    /**
     * @param enabled Is stencil test enabled.
     * @param frontWriteMask Controls which bits are written to in the stencil buffer for front faces.
     * @param frontTest Stencil test settings for the front faces.
     * @param backWriteMask Controls which bits are written to in the stencil buffer for back faces.
     * @param backTest Stencil test settings for the back faces.
     */
    void setStencilTest(bool enabled, uint8 frontWriteMask, StencilTest frontTest, uint8 backWriteMask, StencilTest backTest) {
        SET_FIELD(STENCIL_ACTIVATION, stencilActivation, StencilActivation(enabled, frontWriteMask, backWriteMask));
        SET_FIELD(STENCIL_TEST_FRONT, stencilTestFront, frontTest);
        SET_FIELD(STENCIL_TEST_BACK, stencilTestBack, backTest);
    }

    /**
     * @param enabled Is stencil test enabled.
     * @param frontWriteMask Controls which bits are written to in the stencil buffer for both front and back faces.
     * @param frontTest Stencil test settings for both front and back faces.
     */
    void setStencilTest(bool enabled, uint8 frontWriteMask, StencilTest frontTest) {
        setStencilTest(enabled, frontWriteMask, frontTest, frontWriteMask, frontTest);
    }

    StencilActivation getStencilActivation() const { return _values.stencilActivation; }
    StencilTest getStencilTestFront() const { return _values.stencilTestFront; }
    StencilTest getStencilTestBack() const { return _values.stencilTestBack; }

    bool isStencilEnabled() const { return getStencilActivation().isEnabled(); }
    uint8 getStencilWriteMaskFront() const { return getStencilActivation().getWriteMaskFront(); }
    uint8 getStencilWriteMaskBack() const { return getStencilActivation().getWriteMaskBack(); }

    // Alpha to coverage
    void setAlphaToCoverageEnable(bool enable) { SET_FIELD(ALPHA_TO_COVERAGE_ENABLE, flags.alphaToCoverageEnable, enable); }
    bool isAlphaToCoverageEnabled() const { return _values.flags.alphaToCoverageEnable; }

    // Sample mask
    /**
     * Sets sample mask. Samples that are written to are decided by bit AND of this mask
     * and given fragment's coverage.
     *
     * @param mask Mask that decides which samples can be written to.
     */
    void setSampleMask(uint32 mask) { SET_FIELD(SAMPLE_MASK, sampleMask, mask); }

    /**
     * Gets the current sample mask. Samples that are written to are decided by bit AND of this mask
     * and given fragment's coverage.
     *
     * @return Current sample mask.
     */
    uint32 getSampleMask() const { return _values.sampleMask; }

    // Blend Function
    void setBlendFunction(BlendFunction function) { SET_FIELD(BLEND_FUNCTION, blendFunction, function); }
    const BlendFunction& getBlendFunction() const { return _values.blendFunction; }

    /**
     * @param enabled Is blending enabled?
     * @param sourceColor Determines how source color will be used.
     * @param operationColor Type of operation that will be performed on color if blending is enabled.
     * @param destinationColor Determines how destination color will be used.
     * @param sourceAlpha Determines how source alpha will be used.
     * @param operationAlpha Type of operation that will be performed on alpha if blending is enabled.
     * @param destinationAlpha Determines how destination alpha will be used.
     */
    void setBlendFunction(bool enabled,
                          BlendArg sourceColor,
                          BlendOp operationColor,
                          BlendArg destinationColor,
                          BlendArg sourceAlpha,
                          BlendOp operationAlpha,
                          BlendArg destinationAlpha) {
        setBlendFunction(BlendFunction(enabled, sourceColor, operationColor, destinationColor, sourceAlpha, operationAlpha,
                                       destinationAlpha));
    }

    /**
     * @param enabled Is blending enabled?
     * @param source Determines how source (both color and alpha) will be used.
     * @param operation Type of operation that will be performed on both color and alpha if blending is enabled.
     * @param destination Determines how destination (both color and alpha) will be used.
     */
    void setBlendFunction(bool enabled, BlendArg source, BlendOp operation, BlendArg destination) {
        setBlendFunction(BlendFunction(enabled, source, operation, destination));
    }

    bool isBlendEnabled() const { return getBlendFunction().isEnabled(); }

    // Color write mask
    /**
     * @param mask Bit AND of color channels that are to be written to.
     */
    void setColorWriteMask(ColorMask mask) { SET_FIELD(COLOR_WRITE_MASK, colorWriteMask, mask); }

    /**
     * @brief Sets which color channels can be written to.
     *
     * @param red Should red channel be written to.
     * @param green Should green channel be written to.
     * @param blue Should blue channel be written to.
     * @param alpha Should alpha channel be written to.
     */
    void setColorWriteMask(bool red, bool green, bool blue, bool alpha) {
        ColorMask value = (ColorMask)((WRITE_RED * red) | (WRITE_GREEN * green) | (WRITE_BLUE * blue) | (WRITE_ALPHA * alpha));
        SET_FIELD(COLOR_WRITE_MASK, colorWriteMask, value);
    }
    ColorMask getColorWriteMask() const { return _values.colorWriteMask; }

    /// All the possible fields
    /// NOTE: If you change this, you must update GLBackend::GLState::_resetStateCommands
    enum Field
    {
        FILL_MODE,
        CULL_MODE,
        FRONT_FACE_CLOCKWISE,
        DEPTH_CLAMP_ENABLE,
        SCISSOR_ENABLE,
        MULTISAMPLE_ENABLE,
        ANTIALISED_LINE_ENABLE,

        DEPTH_BIAS,
        DEPTH_BIAS_SLOPE_SCALE,

        DEPTH_TEST,

        STENCIL_ACTIVATION,
        STENCIL_TEST_FRONT,
        STENCIL_TEST_BACK,

        SAMPLE_MASK,

        ALPHA_TO_COVERAGE_ENABLE,

        BLEND_FUNCTION,

        COLOR_WRITE_MASK,

        NUM_FIELDS,  // not a valid field, just the count
    };

    /// The signature of the state tells which fields of the state are not default.
    /// this way during rendering the Backend can compare it's current state and try to minimize the job to do.
    typedef std::bitset<NUM_FIELDS> Signature;

    /**
     * @return Current signature telling which state fields are not default.
     */
    Signature getSignature() const { return _signature; }

    /**
     * @param state State for which singature will be computed.
     * @return Signature telling which state fields are non-default for current state.
     */
    static Signature evalSignature(const Data& state);

    /// For convenience, create a State from the values directly
    State(const Data& values);

    /**
     * @return Object containing all properties of the current state.
     */
    const Data& getValues() const { return _values; }

    /// Object representing state on the API-specific backend side.
    const GPUObjectPointer gpuObject{};

protected:
    State& operator=(const State& state);

    /// Full description of the state.
    Data _values;

    /// Used to tell which fields have default values and which are changed.
    Signature _signature{ 0 };

    /// Incremented on changes and used to determine if API-specific backend object needs updating.
    Stamp _stamp{ 0 };
};

typedef std::shared_ptr<State> StatePointer;
typedef std::vector<StatePointer> States;

};  // namespace gpu

#endif
