//  entityProperties.js
//
//  Created by Ryan Huffman on November 13th, 2014
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022-2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

/* global alert, augmentSpinButtons, clearTimeout, console, document, Element, 
   EventBridge, JSONEditor, openEventBridge, setTimeout, window, _, $ */

var currentTab = "base";

const DEGREES_TO_RADIANS = Math.PI / 180.0;

const UUID_NONE = "{00000000-0000-0000-0000-000000000000}";

const ENTITY_HOST_TYPE_COLOR_DOMAIN = "#afafaf";
const ENTITY_HOST_TYPE_COLOR_AVATAR = "#7fdb98";
const ENTITY_HOST_TYPE_COLOR_LOCAL = "#f0d769";

const NO_SELECTION = ",";

const MAX_TAGS_PER_ROWS = 5;

const AUDIO_ATTENUATION_COEFFICIENT_DEFAULT_VALUE = 0.5;

const PROPERTY_SPACE_MODE = Object.freeze({
    ALL: 0,
    LOCAL: 1,
    WORLD: 2
});

const PROPERTY_SELECTION_VISIBILITY = Object.freeze({
    SINGLE_SELECTION: 1,
    MULTIPLE_SELECTIONS: 2,
    MULTI_DIFF_SELECTIONS: 4,
    ANY_SELECTIONS: 7 /* SINGLE_SELECTION | MULTIPLE_SELECTIONS | MULTI_DIFF_SELECTIONS */
});

// Multiple-selection behavior
const PROPERTY_MULTI_DISPLAY_MODE = Object.freeze({
    DEFAULT: 0,
    /**
     * Comma separated values
     * Limited for properties with type "string" or "textarea" and readOnly enabled
     */
    COMMA_SEPARATED_VALUES: 1
});

const GROUPS = [
    {
        id: "base",
        label: "ENTITY",
        properties: [
            {
                label: NO_SELECTION,
                type: "icon",
                icons: ENTITY_TYPE_ICON,
                propertyID: "type",
                replaceID: "placeholder-property-type",
            },
            {
                label: "Name",
                type: "string",
                propertyID: "name",
                placeholder: "Name",
                replaceID: "placeholder-property-name",
            },
            {
                label: "ID",
                type: "string",
                propertyID: "id",
                placeholder: "ID",
                readOnly: true,
                replaceID: "placeholder-property-id",
                multiDisplayMode: PROPERTY_MULTI_DISPLAY_MODE.COMMA_SEPARATED_VALUES,
            },
            {
                label: "Description",
                type: "string",
                propertyID: "description",
            },
            {
                label: "Parent",
                type: "string",
                propertyID: "parentID",
                buttons: [ { id: "navigateToParentEntity", label: "1", className: "navigation", onClick: navigateToSpecificEntityFromParentID } ],
                onChange: parentIDChanged,
            },
            {
                label: "Parent Joint Index",
                type: "number",
                propertyID: "parentJointIndex",
            },
            {
                label: "",
                glyph: "&#xe006;",
                type: "bool",
                propertyID: "locked",
                replaceID: "placeholder-property-locked",
            },
            {
                label: "",
                glyph: "&#xe007;",
                type: "bool",
                propertyID: "visible",
                replaceID: "placeholder-property-visible",
            },
            {
                label: "Render Layer",
                type: "dropdown",
                options: {
                    world: "World",
                    front: "Front",
                    hud: "HUD"
                },
                propertyID: "renderLayer",
            },
            {
                label: "Primitive Mode",
                type: "dropdown",
                options: {
                    solid: "Solid",
                    lines: "Wireframe",
                },
                propertyID: "primitiveMode",
            },
            {
                label: "Billboard Mode",
                type: "dropdown",
                options: {
                    none: "None",
                    yaw: "Yaw",
                    full: "Full"
                },
                propertyID: "billboardMode",
            },
            {
                label: "Mirror Mode",
                type: "dropdown",
                options: {
                    none: "None",
                    mirror: "Mirror",
                    portal: "Portal"
                },
                propertyID: "mirrorMode",
            },
            {
                label: "Portal Exit",
                type: "string",
                propertyID: "portalExitID",
                showPropertyRule: { "mirrorMode": "portal" },
            },
            {
                label: "Render With Zones",
                type: "multipleZonesSelection",
                propertyID: "renderWithZones",
            },
            {
                label: "Tags",
                type: "arrayOfStrings",
                propertyID: "tags",
                useStringColor: true,
            },
        ]
    },
    {
        id: "shape",
        label: "SHAPE",
        properties: [
            {
                label: "Shape",
                type: "dropdown",
                options: { Cube: "Box", Sphere: "Sphere", Tetrahedron: "Tetrahedron", Octahedron: "Octahedron", 
                           Icosahedron: "Icosahedron", Dodecahedron: "Dodecahedron", Hexagon: "Hexagon", 
                           Triangle: "Triangle", Octagon: "Octagon", Cylinder: "Cylinder", Cone: "Cone", 
                           Circle: "Circle", Quad: "Quad" },
                propertyID: "shape",
            },
            {
                label: "Color",
                type: "color",
                propertyID: "color",
            },
            {
                label: "Alpha",
                type: "number-draggable",
                min: 0,
                max: 1,
                step: 0.01,
                decimals: 2,
                propertyID: "shapeAlpha",
                propertyName: "alpha",
            },
            {
                label: "Unlit",
                type: "bool",
                propertyID: "shapeUnlit",
                propertyName: "unlit"
            }
        ]
    },
    {
        id: "text",
        label: "TEXT",
        properties: [
            {
                label: "Text",
                type: "textarea",
                propertyID: "text",
            },
            {
                label: "Text Color",
                type: "color",
                propertyID: "textColor",
            },
            {
                label: "Text Alpha",
                type: "number-draggable",
                min: 0,
                max: 1,
                step: 0.01,
                decimals: 2,
                propertyID: "textAlpha",
            },
            {
                label: "Background Color",
                type: "color",
                propertyID: "backgroundColor",
            },
            {
                label: "Background Alpha",
                type: "number-draggable",
                min: 0,
                max: 1,
                step: 0.01,
                decimals: 2,
                propertyID: "backgroundAlpha",
            },
            {
                label: "Line Height",
                type: "number-draggable",
                min: 0,
                step: 0.001,
                decimals: 4,
                unit: "m",
                propertyID: "lineHeight",
            },
            {
                label: "Font",
                type: "string",
                propertyID: "font",
            },
            {
                label: "Effect",
                type: "dropdown",
                options: {
                    none: "None",
                    outline: "Outline",
                    "outline fill": "Outline with fill",
                    shadow: "Shadow"
                },
                propertyID: "textEffect",
            },
            {
                label: "Effect Color",
                type: "color",
                propertyID: "textEffectColor",
            },
            {
                label: "Effect Thickness",
                type: "number-draggable",
                min: 0.0,
                max: 0.5,
                step: 0.01,
                decimals: 2,
                propertyID: "textEffectThickness",
            },
            {
                label: "Alignment",
                type: "dropdown",
                options: {
                    left: "Left",
                    center: "Center",
                    right: "Right"
                },
                propertyID: "textAlignment",
                propertyName: "alignment", // actual entity property name
            },
            {
                label: "Vertical Alignment",
                type: "dropdown",
                options: {
                    top: "Top",
                    center: "Center",
                    bottom: "Bottom"
                },
                propertyID: "textVerticalAlignment",
                propertyName: "verticalAlignment", // actual entity property name
            },
            {
                label: "Top Margin",
                type: "number-draggable",
                step: 0.01,
                decimals: 2,
                propertyID: "topMargin",
            },
            {
                label: "Right Margin",
                type: "number-draggable",
                step: 0.01,
                decimals: 2,
                propertyID: "rightMargin",
            },
            {
                label: "Bottom Margin",
                type: "number-draggable",
                step: 0.01,
                decimals: 2,
                propertyID: "bottomMargin",
            },
            {
                label: "Left Margin",
                type: "number-draggable",
                step: 0.01,
                decimals: 2,
                propertyID: "leftMargin",
            },
            {
                label: "Unlit",
                type: "bool",
                propertyID: "unlit",
            }
        ]
    },
    {
        id: "zone",
        label: "ZONE",
        properties: [
            {
                label: "Shape Type",
                type: "dropdown",
                options: { "box": "Box", "sphere": "Sphere",
                           "cylinder-y": "Cylinder", "compound": "Use Compound Shape URL" },
                propertyID: "zoneShapeType",
                propertyName: "shapeType", // actual entity property name
            },
            {
                label: "Compound Shape URL",
                type: "string",
                propertyID: "zoneCompoundShapeURL",
                propertyName: "compoundShapeURL", // actual entity property name
                placeholder: "URL",
            },
            {
                label: "Flying Allowed",
                type: "bool",
                propertyID: "flyingAllowed",
            },
            {
                label: "Ghosting Allowed",
                type: "bool",
                propertyID: "ghostingAllowed",
            },
            {
                label: "Filter",
                type: "string",
                propertyID: "filterURL",
                placeholder: "URL",
            }
        ]
    },
    {
        id: "zone_key_light",
        label: "ZONE KEY LIGHT",
        properties: [
            {
                label: "Key Light",
                type: "dropdown",
                options: { inherit: "Inherit", disabled: "Off", enabled: "On" },
                propertyID: "keyLightMode",
                
            },
            {
                label: "Key Light Color",
                type: "color",
                propertyID: "keyLight.color",
                showPropertyRule: { "keyLightMode": "enabled" },
            },
            {
                label: "Light Intensity",
                type: "number-draggable",
                min: -40,
                max: 40,
                step: 0.01,
                decimals: 2,
                propertyID: "keyLight.intensity",
                showPropertyRule: { "keyLightMode": "enabled" },
            },
            {
                label: "Light Horizontal Angle",
                type: "number-draggable",
                step: 0.1,
                multiplier: DEGREES_TO_RADIANS,
                decimals: 2,
                unit: "deg",
                propertyID: "keyLight.direction.y",
                showPropertyRule: { "keyLightMode": "enabled" },
            },
            {
                label: "Light Vertical Angle",
                type: "number-draggable",
                step: 0.1,
                multiplier: DEGREES_TO_RADIANS,
                decimals: 2,
                unit: "deg",
                propertyID: "keyLight.direction.x",
                showPropertyRule: { "keyLightMode": "enabled" },
            },
            {
                label: "Cast Shadows",
                type: "bool",
                propertyID: "keyLight.castShadows",
                showPropertyRule: { "keyLightMode": "enabled" },
            },
            {
                label: "Shadow Bias",
                type: "number-draggable",
                min: 0,
                max: 1,
                step: 0.01,
                decimals: 2,
                propertyID: "keyLight.shadowBias",
                showPropertyRule: { "keyLightMode": "enabled" },
            },
            {
                label: "Shadow Max Distance",
                type: "number-draggable",
                min: 0,
                max: 250,
                step: 0.1,
                decimals: 2,
                propertyID: "keyLight.shadowMaxDistance",
                showPropertyRule: { "keyLightMode": "enabled" },
            }
        ]
    },    
    {
        id: "zone_skybox",
        label: "ZONE SKYBOX",
        properties: [
            {
                label: "Skybox",
                type: "dropdown",
                options: { inherit: "Inherit", disabled: "Off", enabled: "On" },
                propertyID: "skyboxMode",
            },
            {
                label: "Skybox Color",
                type: "color",
                propertyID: "skybox.color",
                showPropertyRule: { "skyboxMode": "enabled" },
            },
            {
                label: "Skybox Source",
                type: "string",
                propertyID: "skybox.url",
                showPropertyRule: { "skyboxMode": "enabled" },
                placeholder: "URL",
            }
        ]
    },
    {
        id: "zone_ambient_light",
        label: "ZONE AMBIENT LIGHT",
        properties: [
            {
                label: "Ambient Light",
                type: "dropdown",
                options: { inherit: "Inherit", disabled: "Off", enabled: "On" },
                propertyID: "ambientLightMode",
            },
            {
                label: "Ambient Color",
                type: "color",
                propertyID: "ambientLight.ambientColor",
                showPropertyRule: { "ambientLightMode": "enabled" },
            },
            {
                type: "buttons",
                buttons: [ { id: "copy", label: "Copy color from Skybox", 
                             className: "black", onClick: copySkyboxColorToAmbientColor } ],
                propertyID: "copyColorToAmbient",
                showPropertyRule: { "ambientLightMode": "enabled" },
            },
            {
                label: "Ambient Intensity",
                type: "number-draggable",
                min: -200,
                max: 200,
                step: 0.1,
                decimals: 2,
                propertyID: "ambientLight.ambientIntensity",
                showPropertyRule: { "ambientLightMode": "enabled" },
            },
            {
                label: "Ambient Source",
                type: "string",
                propertyID: "ambientLight.ambientURL",
                showPropertyRule: { "ambientLightMode": "enabled" },
                placeholder: "URL",
            },
            {
                type: "buttons",
                buttons: [ { id: "copy", label: "Copy URL from Skybox", 
                             className: "black", onClick: copySkyboxURLToAmbientURL } ],
                propertyID: "copyURLToAmbient",
                showPropertyRule: { "ambientLightMode": "enabled" },
            }
        ]
    },
    {
        id: "zone_haze",
        label: "ZONE HAZE",
        properties: [
            {
                label: "Haze",
                type: "dropdown",
                options: { inherit: "Inherit", disabled: "Off", enabled: "On" },
                propertyID: "hazeMode",
            },
            {
                label: "Range",
                type: "number-draggable",
                min: 1,
                max: 10000,
                step: 1,
                decimals: 0,
                unit: "m",
                propertyID: "haze.hazeRange",
                showPropertyRule: { "hazeMode": "enabled" },
            },
            {
                label: "Use Altitude",
                type: "bool",
                propertyID: "haze.hazeAltitudeEffect",
                showPropertyRule: { "hazeMode": "enabled" },
            },
            {
                label: "Base",
                type: "number-draggable",
                min: -16000,
                max: 16000,
                step: 1,
                decimals: 0,
                unit: "m",
                propertyID: "haze.hazeBaseRef",
                showPropertyRule: { "hazeMode": "enabled" },
            },
            {
                label: "Ceiling",
                type: "number-draggable",
                min: -16000,
                max: 16000,
                step: 1,
                decimals: 0,
                unit: "m",
                propertyID: "haze.hazeCeiling",
                showPropertyRule: { "hazeMode": "enabled" },
            },
            {
                label: "Haze Color",
                type: "color",
                propertyID: "haze.hazeColor",
                showPropertyRule: { "hazeMode": "enabled" },
            },
            {
                label: "Background Blend",
                type: "number-draggable",
                min: 0,
                max: 1,
                step: 0.001,
                decimals: 3,
                propertyID: "haze.hazeBackgroundBlend",
                showPropertyRule: { "hazeMode": "enabled" },
            },
            {
                label: "Enable Glare",
                type: "bool",
                propertyID: "haze.hazeEnableGlare",
                showPropertyRule: { "hazeMode": "enabled" },
            },
            {
                label: "Glare Color",
                type: "color",
                propertyID: "haze.hazeGlareColor",
                showPropertyRule: { "hazeMode": "enabled" },
            },
            {
                label: "Glare Angle",
                type: "number-draggable",
                min: 0,
                max: 180,
                step: 1,
                decimals: 0,
                propertyID: "haze.hazeGlareAngle",
                showPropertyRule: { "hazeMode": "enabled" },
            }
        ]
    },
    {
        id: "zone_bloom",
        label: "ZONE BLOOM",
        properties: [
            {
                label: "Bloom",
                type: "dropdown",
                options: { inherit: "Inherit", disabled: "Off", enabled: "On" },
                propertyID: "bloomMode",
            },
            {
                label: "Bloom Intensity",
                type: "number-draggable",
                min: 0,
                max: 1,
                step: 0.001,
                decimals: 3,
                propertyID: "bloom.bloomIntensity",
                showPropertyRule: { "bloomMode": "enabled" },
            },
            {
                label: "Bloom Threshold",
                type: "number-draggable",
                min: 0,
                max: 1,
                step: 0.001,
                decimals: 3,
                propertyID: "bloom.bloomThreshold",
                showPropertyRule: { "bloomMode": "enabled" },
            },
            {
                label: "Bloom Size",
                type: "number-draggable",
                min: 0,
                max: 2,
                step: 0.001,
                decimals: 3,
                propertyID: "bloom.bloomSize",
                showPropertyRule: { "bloomMode": "enabled" },
            }
        ]
    },
    {
        id: "zone_tonemapping",
        label: "ZONE TONEMAPPING",
        properties: [
            {
                label: "Tonemapping",
                type: "dropdown",
                options: { inherit: "Inherit", disabled: "Off", enabled: "On" },
                propertyID: "tonemappingMode",
            },
            {
                label: "Curve",
                type: "dropdown",
                options: { rgb: "RGB", srgb: "sRGB", reinhard: "Reinhard", filmic: "Filmic" },
                propertyID: "tonemapping.curve",
                showPropertyRule: { "tonemappingMode": "enabled" },
            },
            {
                label: "Exposure",
                type: "number-draggable",
                min: -4.0,
                max: 4.0,
                step: 0.1,
                decimals: 1,
                propertyID: "tonemapping.exposure",
                showPropertyRule: { "tonemappingMode": "enabled" },
            }
        ]
    },
    {
        id: "zone_ambient_occlusion",
        label: "ZONE AMBIENT OCCLUSION",
        properties: [
            {
                label: "Ambient Occlusion",
                type: "dropdown",
                options: { inherit: "Inherit", disabled: "Off", enabled: "On" },
                propertyID: "ambientOcclusionMode",
            },
            //{
            //    label: "Technique",
            //    type: "dropdown",
            //    options: { ssao: "SSAO", hbao: "HBAO" },
            //    propertyID: "ambientOcclusion.technique",
            //    showPropertyRule: { "ambientOcclusionMode": "enabled" },
            //},
            {
                label: "Jitter",
                type: "bool",
                propertyID: "ambientOcclusion.jitter",
                showPropertyRule: { "ambientOcclusionMode": "enabled" },
            },
            {
                label: "Resolution Level",
                type: "number-draggable",
                step: 1,
                decimals: 0,
                propertyID: "ambientOcclusion.resolutionLevel",
                showPropertyRule: { "ambientOcclusionMode": "enabled" },
            },
            {
                label: "Edge Sharpness",
                type: "number-draggable",
                step: 0.01,
                decimals: 2,
                propertyID: "ambientOcclusion.edgeSharpness",
                showPropertyRule: { "ambientOcclusionMode": "enabled" },
            },
            {
                label: "Blur Radius (pixels)",
                type: "number-draggable",
                step: 1,
                decimals: 0,
                propertyID: "ambientOcclusion.blurRadius",
                showPropertyRule: { "ambientOcclusionMode": "enabled" },
            },
            {
                label: "AO Radius",
                type: "number-draggable",
                step: 0.01,
                decimals: 2,
                propertyID: "ambientOcclusion.aoRadius",
                showPropertyRule: { "ambientOcclusionMode": "enabled" },
            },
            {
                label: "Intensity",
                type: "number-draggable",
                step: 0.01,
                decimals: 2,
                propertyID: "ambientOcclusion.aoObscuranceLevel",
                showPropertyRule: { "ambientOcclusionMode": "enabled" },
            },
            {
                label: "Falloff Angle",
                type: "number-draggable",
                step: 0.01,
                decimals: 2,
                propertyID: "ambientOcclusion.aoFalloffAngle",
                showPropertyRule: { "ambientOcclusionMode": "enabled" },
            },
            {
                label: "Sampling Amount",
                type: "number-draggable",
                step: 0.01,
                decimals: 2,
                propertyID: "ambientOcclusion.aoSamplingAmount",
                showPropertyRule: { "ambientOcclusionMode": "enabled" },
            },
            {
                label: "Num Spiral Turns",
                type: "number-draggable",
                step: 0.01,
                decimals: 2,
                propertyID: "ambientOcclusion.ssaoNumSpiralTurns",
                showPropertyRule: { "ambientOcclusionMode": "enabled" },
            }
        ]
    },
    {
        id: "zone_avatar_priority",
        label: "ZONE AVATAR PRIORITY",
        properties: [
            {
                label: "Avatar Priority",
                type: "dropdown",
                options: { inherit: "Inherit", crowd: "Crowd", hero: "Hero" },
                propertyID: "avatarPriority",
            }
        ]
    },
    {
        id: "zone_audio",
        label: "ZONE AUDIO",
        properties: [
            {
                label: "Enable Reverb",
                type: "bool",
                propertyID: "audio.reverbEnabled"
            },
            {
                label: "Reverb Time",
                type: "number-draggable",
                min: 0,
                max: 10,
                step: 0.1,
                decimals: 2,
                propertyID: "audio.reverbTime",
                showPropertyRule: { "audio.reverbEnabled": "true" },
            },
            {
                label: "Reverb Wet Level",
                type: "number-draggable",
                min: 0,
                max: 100,
                step: 1,
                decimals: 1,
                propertyID: "audio.reverbWetLevel",
                showPropertyRule: { "audio.reverbEnabled": "true" },
            },
            {
                label: "Listener Zones",
                type: "multipleZonesSelection",
                propertyID: "audio.listenerZones",
                zonesCoefficientID: "audio.listenerAttenuationCoefficients",
            },
            {
                label: "Attenuation Coef.",
                type: "zonesCoefficient",
                propertyID: "audio.listenerAttenuationCoefficients",
                multipleZonesSelectionID: "audio.listenerZones",
            }
        ]
    },
    {
        id: "model",
        label: "MODEL",
        properties: [
            {
                label: "Model",
                type: "string",
                placeholder: "URL",
                propertyID: "modelURL",
            },
            {
                label: "Collision Shape",
                type: "dropdown",
                options: { "none": "No Collision", "box": "Box", "sphere": "Sphere", "compound": "Compound" , 
                           "simple-hull": "Basic - Whole model", "simple-compound": "Good - Sub-meshes" , 
                           "static-mesh": "Exact - All polygons (non-dynamic only)" },
                propertyID: "shapeType",
            },
            {
                label: "Compound Shape",
                type: "string",
                propertyID: "compoundShapeURL",
                placeholder: "URL",
            },
            {
                label: "Use Original Pivot",
                type: "bool",
                propertyID: "useOriginalPivot",
            },
            {
                label: "Animation",
                type: "string",
                propertyID: "animation.url",
                placeholder: "URL",
            },
            {
                label: "Play Automatically",
                type: "bool",
                propertyID: "animation.running",
            },
            {
                label: "Loop",
                type: "bool",
                propertyID: "animation.loop",
            },
            {
                label: "Allow Translation",
                type: "bool",
                propertyID: "animation.allowTranslation",
            },
            {
                label: "Hold",
                type: "bool",
                propertyID: "animation.hold",
            },
            {
                label: "Animation Frame",
                type: "number-draggable",
                propertyID: "animation.currentFrame",
            },
            {
                label: "First Frame",
                type: "number-draggable",
                propertyID: "animation.firstFrame",
            },
            {
                label: "Last Frame",
                type: "number-draggable",
                propertyID: "animation.lastFrame",
            },
            {
                label: "Animation FPS",
                type: "number-draggable",
                propertyID: "animation.fps",
            },
            {
                label: "Smooth Animation",
                type: "bool",
                propertyID: "animation.smoothFrames",
            },
            {
                label: "Texture",
                type: "textarea",
                propertyID: "textures",
            },
            {
                label: "Original Texture",
                type: "textarea",
                propertyID: "originalTextures",
                readOnly: true,
            },
            {
                label: "Group Culled",
                type: "bool",
                propertyID: "groupCulled",
            }
        ]
    },
    {
        id: "image",
        label: "IMAGE",
        properties: [
            {
                label: "Image",
                type: "string",
                placeholder: "URL",
                propertyID: "imageURL",
            },
            {
                label: "Color",
                type: "color",
                propertyID: "imageColor",
                propertyName: "color", // actual entity property name
            },
            {
                label: "Alpha",
                type: "number-draggable",
                min: 0,
                max: 1,
                step: 0.01,
                decimals: 2,
                propertyID: "imageAlpha",
                propertyName: "alpha",
            },            
            {
                label: "Emissive",
                type: "bool",
                propertyID: "emissive",
            },
            {
                label: "Sub Image",
                type: "rect",
                min: 0,
                step: 1,
                subLabels: [ "x", "y", "w", "h" ],
                propertyID: "subImage",
            },
            {
                label: "Keep Aspect Ratio",
                type: "bool",
                propertyID: "keepAspectRatio",
            }
        ]
    },
    {
        id: "web",
        label: "WEB",
        properties: [
            {
                label: "Source",
                type: "string",
                propertyID: "sourceUrl",
                placeholder: "URL",
            },
            {
                label: "Source Resolution",
                type: "number-draggable",
                propertyID: "dpi",
            },
            {
                label: "Web Color",
                type: "color",
                propertyID: "webColor",
                propertyName: "color", // actual entity property name
            },
            {
                label: "Web Alpha",
                type: "number-draggable",
                step: 0.001,
                decimals: 3,
                propertyID: "webAlpha",
                propertyName: "alpha",
                min: 0,
                max: 1,
            },
            {
                label: "Use Background",
                type: "bool",
                propertyID: "useBackground",
            },
            {
                label: "Max FPS",
                type: "number-draggable",
                step: 1,
                decimals: 0,
                propertyID: "maxFPS",
            },
            {
                label: "Input Mode",
                type: "dropdown",
                options: {
                    touch: "Touch events",
                    mouse: "Mouse events"
                },
                propertyID: "inputMode",
            },
            {
                label: "Wants Keyboard Focus",
                type: "bool",
                propertyID: "wantsKeyboardFocus",
            },
            {
                label: "Focus Highlight",
                type: "bool",
                propertyID: "showKeyboardFocusHighlight",
            },            
            {
                label: "Script URL",
                type: "string",
                propertyID: "scriptURL",
                placeholder: "URL",
            },
            {
                label: "User Agent",
                type: "string",
                propertyID: "userAgent",
                placeholder: "User Agent",
            }
        ]
    },
    {
        id: "light",
        label: "LIGHT",
        properties: [
            {
                label: "Light Color",
                type: "color",
                propertyID: "lightColor",
                propertyName: "color", // actual entity property name
            },
            {
                label: "Intensity",
                type: "number-draggable",
                min: -1000,
                max: 10000,
                step: 0.1,
                decimals: 2,
                propertyID: "intensity",
            },
            {
                label: "Fall-Off Radius",
                type: "number-draggable",
                min: 0,
                max: 10000,
                step: 0.1,
                decimals: 2,
                unit: "m",
                propertyID: "falloffRadius",
            },
            {
                label: "Spotlight",
                type: "bool",
                propertyID: "isSpotlight",
            },
            {
                label: "Spotlight Exponent",
                type: "number-draggable",
                min: 0,
                step: 0.01,
                decimals: 2,
                propertyID: "exponent",
            },
            {
                label: "Spotlight Cut-Off",
                type: "number-draggable",
                step: 0.01,
                decimals: 2,
                propertyID: "cutoff",
            }
        ]
    },
    {
        id: "material",
        label: "MATERIAL",
        properties: [
            {
                label: "Material URL",
                type: "string",
                propertyID: "materialURL",
                placeholder: "URL",
            },
            {
                label: "Material Data",
                type: "textarea",
                buttons: [ { id: "materialAssistant", label: "Assistant...", className: "secondary_blue blue", onClick: openMaterialAssistant },
                           { id: "clear", label: "Clear Material", className: "secondary_red red", onClick: clearMaterialData }, 
                           { id: "edit", label: "Edit as JSON", className: "secondary", onClick: newJSONMaterialEditor },
                           { id: "save", label: "Save Material", className: "secondary", onClick: saveMaterialData }],
                propertyID: "materialData",
            },
            {
                label: "Material Target",
                type: "dynamic-multiselect",
                propertyUpdate: materialTargetPropertyUpdate,
                propertyID: "parentMaterialName",
                selectionVisibility: PROPERTY_SELECTION_VISIBILITY.SINGLE_SELECTION,
            },
            {
                label: "Priority",
                type: "number-draggable",
                min: 0,
                propertyID: "priority",
            },
            {
                label: "Material Mapping Mode",
                type: "dropdown",
                options: {
                    uv: "UV space", projected: "3D projected", triplanar: "Triplanar mapping"
                },
                propertyID: "materialMappingMode",
            },
            {
                label: "Material Position",
                type: "vec2",
                vec2Type: "xyz",
                min: 0,
                max: 1,
                step: 0.005,
                decimals: 4,
                subLabels: [ "x", "y" ],
                propertyID: "materialMappingPos",
            },
            {
                label: "Material Scale",
                type: "vec2",
                vec2Type: "xyz",
                step: 0.005,
                decimals: 4,
                subLabels: [ "x", "y" ],
                propertyID: "materialMappingScale",
            },
            {
                label: "Material Rotation",
                type: "number-draggable",
                step: 0.1,
                decimals: 2,
                unit: "deg",
                propertyID: "materialMappingRot",
            },
            {
                label: "Material Repeat",
                type: "bool",
                propertyID: "materialRepeat",
            }
        ]
    },
    {
        id: "grid",
        label: "GRID",
        properties: [
            {
                label: "Color",
                type: "color",
                propertyID: "gridColor",
                propertyName: "color", // actual entity property name
            },
            {
                label: "Follow Camera",
                type: "bool",
                propertyID: "followCamera",
            },
            {
                label: "Major Grid Every",
                type: "number-draggable",
                min: 0,
                step: 1,
                decimals: 0,
                propertyID: "majorGridEvery",
            },
            {
                label: "Minor Grid Every",
                type: "number-draggable",
                min: 0,
                step: 0.01,
                decimals: 2,
                propertyID: "minorGridEvery",
            }
        ]
    },
    {
        id: "particles",
        label: "PARTICLES",
        properties: [
            {
                label: "Emit",
                type: "bool",
                propertyID: "isEmitting",
            },
            {
                label: "Lifespan",
                type: "number-draggable",
                unit: "s",
                step: 0.01,
                decimals: 2,
                propertyID: "lifespan",
            },
            {
                label: "Max Particles",
                type: "number-draggable",
                step: 1,
                propertyID: "maxParticles",
            },
            {
                label: "Texture",
                type: "texture",
                propertyID: "particleTextures",
                propertyName: "textures", // actual entity property name
            }
        ]
    },
    {
        id: "particles_emit",
        label: "PARTICLES EMIT",
        properties: [
            {
                label: "Emit Rate",
                type: "number-draggable",
                step: 1,
                propertyID: "emitRate",
            },
            {
                label: "Emit Speed",
                type: "number-draggable",
                step: 0.1,
                decimals: 2,
                propertyID: "emitSpeed",
            },
            {
                label: "Speed Spread",
                type: "number-draggable",
                step: 0.1,
                decimals: 2,
                propertyID: "speedSpread",
            },
            {
                label: "Shape Type",
                type: "dropdown",
                options: { "box": "Box", "ellipsoid": "Ellipsoid", 
                           "cylinder-y": "Cylinder", "circle": "Circle", "plane": "Plane",
                           "compound": "Use Compound Shape URL" },
                propertyID: "particleShapeType",
                propertyName: "shapeType",
            },
            {
                label: "Compound Shape URL",
                type: "string",
                propertyID: "particleCompoundShapeURL",
                propertyName: "compoundShapeURL",
                placeholder: "URL",
            },
            {
                label: "Emit Dimensions",
                type: "vec3",
                vec3Type: "xyz",
                step: 0.01,
                round: 100,
                subLabels: [ "x", "y", "z" ],
                propertyID: "emitDimensions",
            },
            {
                label: "Emit Radius Start",
                type: "number-draggable",
                step: 0.001,
                decimals: 3,
                propertyID: "emitRadiusStart"
            },
            {
                label: "Emit Orientation",
                type: "vec3",
                vec3Type: "pyr",
                step: 0.01,
                round: 100,
                subLabels: [ "x", "y", "z" ],
                unit: "deg",
                propertyID: "emitOrientation",
            },
            {
                label: "Trails",
                type: "bool",
                propertyID: "emitterShouldTrail",
            }
        ]
    },
    {
        id: "particles_size",
        label: "PARTICLES SIZE",
        properties: [
            {
                type: "triple",
                label: "Size",
                propertyID: "particleRadiusTriple",
                properties: [
                    {
                        label: "Start",
                        type: "number-draggable",
                        step: 0.01,
                        decimals: 2,
                        propertyID: "radiusStart",
                        fallbackProperty: "particleRadius",
                    },
                    {
                        label: "Middle",
                        type: "number-draggable",
                        step: 0.01,
                        decimals: 2,
                        propertyID: "particleRadius",
                    },
                    {
                        label: "Finish",
                        type: "number-draggable",
                        step: 0.01,
                        decimals: 2,
                        propertyID: "radiusFinish",
                        fallbackProperty: "particleRadius",
                    }
                ]
            },
            {
                label: "Size Spread",
                type: "number-draggable",
                step: 0.01,
                decimals: 2,
                propertyID: "radiusSpread",
            }
        ]
    },
    {
        id: "particles_color",
        label: "PARTICLES COLOR",
        properties: [
            {
                type: "triple",
                label: "Color",
                propertyID: "particleColorTriple",
                properties: [
                    {
                        label: "Start",
                        type: "color",
                        propertyID: "colorStart",
                        fallbackProperty: "color",
                    },
                    {
                        label: "Middle",
                        type: "color",
                        propertyID: "particleColor",
                        propertyName: "color", // actual entity property name
                    },
                    {
                        label: "Finish",
                        type: "color",
                        propertyID: "colorFinish",
                        fallbackProperty: "color",
                    }
                ]
            },
            {
                label: "Color Spread",
                type: "vec3rgb",
                vec3Type: "vec3rgb",
                min: 0,
                max: 255,
                step: 1,       
                decimals: 0,
                subLabels: [ "r", "g", "b" ],
                propertyID: "colorSpread",
            },
            {
                type: "triple",
                label: "Alpha",
                propertyID: "particleAlphaTriple",
                properties: [
                    {
                        label: "Start",
                        type: "number-draggable",
                        step: 0.001,
                        decimals: 3,
                        propertyID: "alphaStart",
                        fallbackProperty: "alpha",
                    },
                    {
                        label: "Middle",
                        type: "number-draggable",
                        step: 0.001,
                        decimals: 3,
                        propertyID: "alpha",
                    },
                    {
                        label: "Finish",
                        type: "number-draggable",
                        step: 0.001,
                        decimals: 3,
                        propertyID: "alphaFinish",
                        fallbackProperty: "alpha",
                    }
                ]
            },
            {
                label: "Alpha Spread",
                type: "number-draggable",
                step: 0.001,
                decimals: 3,
                propertyID: "alphaSpread",
            }
        ]
    },
    {
        id: "particles_behavior",
        label: "PARTICLES BEHAVIOR",
        properties: [
            {
                label: "Emit Acceleration",
                type: "vec3",
                vec3Type: "xyz",
                step: 0.01,
                round: 100,
                subLabels: [ "x", "y", "z" ],
                propertyID: "emitAcceleration",
            },
            {
                label: "Acceleration Spread",
                type: "vec3",
                vec3Type: "xyz",
                step: 0.01,
                round: 100,
                subLabels: [ "x", "y", "z" ],
                propertyID: "accelerationSpread",
            },
            {
                type: "triple",
                label: "Spin",
                propertyID: "particleSpinTriple",
                properties: [
                    {
                        label: "Start",
                        type: "number-draggable",
                        step: 0.1,
                        decimals: 2,
                        multiplier: DEGREES_TO_RADIANS,
                        unit: "deg",
                        propertyID: "spinStart",
                        fallbackProperty: "particleSpin",
                    },
                    {
                        label: "Middle",
                        type: "number-draggable",
                        step: 0.1,
                        decimals: 2,
                        multiplier: DEGREES_TO_RADIANS,
                        unit: "deg",
                        propertyID: "particleSpin",
                    },
                    {
                        label: "Finish",
                        type: "number-draggable",
                        step: 0.1,
                        decimals: 2,
                        multiplier: DEGREES_TO_RADIANS,
                        unit: "deg",
                        propertyID: "spinFinish",
                        fallbackProperty: "particleSpin",
                    }
                ]
            },
            {
                label: "Spin Spread",
                type: "number-draggable",
                step: 0.1,
                decimals: 2,
                multiplier: DEGREES_TO_RADIANS,
                unit: "deg",
                propertyID: "spinSpread",
            },
            {
                label: "Rotate with Entity",
                type: "bool",
                propertyID: "rotateWithEntity",
            }
        ]
    },
    {
        id: "particles_constraints",
        label: "PARTICLES CONSTRAINTS",
        properties: [
            {
                type: "triple",
                label: "Horizontal Angle",
                propertyID: "particlePolarTriple",
                properties: [
                    {
                        label: "Start",
                        type: "number-draggable",
                        step: 0.1,
                        decimals: 2,
                        multiplier: DEGREES_TO_RADIANS,
                        unit: "deg",
                        propertyID: "polarStart",
                    },
                    {
                        label: "Finish",
                        type: "number-draggable",
                        step: 0.1,
                        decimals: 2,
                        multiplier: DEGREES_TO_RADIANS,
                        unit: "deg",
                        propertyID: "polarFinish",
                    }
                ],
            },
            {
                type: "triple",
                label: "Vertical Angle",
                propertyID: "particleAzimuthTriple",
                properties: [
                    {
                        label: "Start",
                        type: "number-draggable",
                        step: 0.1,
                        decimals: 2,
                        multiplier: DEGREES_TO_RADIANS,
                        unit: "deg",
                        propertyID: "azimuthStart",
                    },
                    {
                        label: "Finish",
                        type: "number-draggable",
                        step: 0.1,
                        decimals: 2,
                        multiplier: DEGREES_TO_RADIANS,
                        unit: "deg",
                        propertyID: "azimuthFinish",
                    }
                ]
            }
        ]
    },
    {
        id: "particles_procedural",
        label: "PROCEDURAL PARTICLES",
        properties: [
            {
                label: "Particles",
                type: "number-draggable",
                propertyID: "numParticles",
                min: 1,
                max: 1000000
            },
            {
                label: "Triangles Per Particle",
                type: "number-draggable",
                propertyID: "numTrianglesPerParticle",
                min: 1,
                max: 15
            },
            {
                label: "Update Props",
                type: "number-draggable",
                propertyID: "numUpdateProps",
                min: 0,
                max: 5
            },
            {
                label: "Transparent",
                type: "bool",
                propertyID: "particleTransparent",
            },
            {
                label: "Particle Update Data",
                type: "textarea",
                buttons: [{ id: "clear", label: "Clear Update Data", className: "secondary_red red", onClick: clearParticleUpdateData },
                { id: "edit", label: "Edit as JSON", className: "secondary", onClick: newJSONParticleUpdateEditor },
                { id: "save", label: "Save Update Data", className: "secondary", onClick: saveParticleUpdateData }],
                propertyID: "particleUpdateData",
            },
            {
                label: "Particle Render Data",
                type: "textarea",
                buttons: [{ id: "clear", label: "Clear Render Data", className: "secondary_red red", onClick: clearParticleRenderData },
                { id: "edit", label: "Edit as JSON", className: "secondary", onClick: newJSONParticleRenderEditor },
                { id: "save", label: "Save Render Data", className: "secondary", onClick: saveParticleRenderData }],
                propertyID: "particleRenderData",
            }
        ]
    },
    {
        id: "polyvox",
        label: "POLYVOX",
        properties: [
            {
                label: "Volume Size",
                type: "vec3",
                vec3Type: "xyz",
                step: 1.0,
                decimals: 0,
                subLabels: [ "x", "y", "z" ],
                unit: "",
                propertyID: "voxelVolumeSize",
            },
            {
                label: "Surface Style",
                type: "dropdown",
                options: { 0: "Marching cubes", 1: "Cubic", 
                           2: "Edged cubic", 3: "Edged marching cubes" },
                propertyID: "voxelSurfaceStyle",
                propertyName: "voxelSurfaceStyle",
            },
            {
                label: "Texture preset",
                type: "dropdown",
                options: { 0 : "None", 1 : "Grass + ground", 2 : "Bricks", 3 : "Stone", 
                           4: "Concrete", 5 : "Rock"},
                propertyID: "polyVoxPreset",
                onDropdownChange: createPolyVoxPresetChangedFunction,
                skipPropertyUpdate: true,
            },
            {
                label: "X Texture URL",
                type: "string",
                propertyID: "xTextureURL",
                propertyName: "xTextureURL",
                placeholder: "URL",
            },
            {
                label: "Y Texture URL",
                type: "string",
                propertyID: "yTextureURL",
                propertyName: "yTextureURL",
                placeholder: "URL",
            },
            {
                label: "Z Texture URL",
                type: "string",
                propertyID: "zTextureURL",
                propertyName: "zTextureURL",
                placeholder: "URL",
            },
        ]
    },
    {
        id: "sound",
        label: "SOUND",
        properties: [
            {
                label: "Sound",
                type: "string",
                propertyID: "soundURL",
                placeholder: "URL",
            },
            {
                label: "Playing",
                type: "bool",
                propertyID: "playing",
            },
            {
                label: "Loop",
                type: "bool",
                propertyID: "loop",
            },
            {
                label: "Volume",
                type: "number-draggable",
                min: 0,
                max: 1,
                step: 0.01,
                decimals: 2,
                propertyID: "volume",
            },
            {
                label: "Positional",
                type: "bool",
                propertyID: "positional",
            },
            {
                label: "Pitch",
                type: "number-draggable",
                min: 0.0625,
                max: 16,
                step: 0.1,
                decimals: 2,
                propertyID: "pitch",
            },
            {
                label: "Time Offset",
                type: "number-draggable",
                step: 0.1,
                propertyID: "timeOffset",
            },
            {
                label: "Local Only",
                type: "bool",
                propertyID: "localOnly",
            }
        ]
    },
    {
        id: "spatial",
        label: "SPATIAL",
        properties: [
            {
                label: "Position",
                type: "vec3",
                vec3Type: "xyz",
                step: 0.1,
                decimals: 4,
                subLabels: [ "x", "y", "z" ],
                unit: "m",
                propertyID: "position",
                spaceMode: PROPERTY_SPACE_MODE.WORLD,
            },
            {
                label: "Local Position",
                type: "vec3",
                vec3Type: "xyz",
                step: 0.1,
                decimals: 4,
                subLabels: [ "x", "y", "z" ],
                unit: "m",
                propertyID: "localPosition",
                spaceMode: PROPERTY_SPACE_MODE.LOCAL,
            },
            {
                type: "buttons",
                buttons: [  { id: "copyPosition", label: "Copy Position", className: "secondary", onClick: copyPositionProperty },
                            { id: "pastePosition", label: "Paste Position", className: "secondary", onClick: pastePositionProperty } ],
                propertyID: "copyPastePosition"
            },
            {
                label: "Rotation",
                type: "vec3",
                vec3Type: "pyr",
                step: 0.1,
                decimals: 4,
                subLabels: [ "x", "y", "z" ],
                unit: "deg",
                propertyID: "rotation",
                spaceMode: PROPERTY_SPACE_MODE.WORLD,
            },
            {
                label: "Local Rotation",
                type: "vec3",
                vec3Type: "pyr",
                step: 0.1,
                decimals: 4,
                subLabels: [ "x", "y", "z" ],
                unit: "deg",
                propertyID: "localRotation",
                spaceMode: PROPERTY_SPACE_MODE.LOCAL,
            },
            {
                type: "buttons",
                buttons: [  { id: "copyRotation", label: "Copy Rotation", className: "secondary", onClick: copyRotationProperty },
                            { id: "pasteRotation", label: "Paste Rotation", className: "secondary", onClick: pasteRotationProperty },
                            { id: "setRotationToZero", label: "Reset Rotation", className: "secondary_red red", onClick: setRotationToZeroProperty }],
                propertyID: "copyPasteRotation"
            },          
            {
                label: "Dimensions",
                type: "vec3",
                vec3Type: "xyz",
                step: 0.01,
                decimals: 4,
                subLabels: [ "x", "y", "z" ],
                unit: "m",
                propertyID: "dimensions",
                spaceMode: PROPERTY_SPACE_MODE.WORLD,
            },
            {
                label: "Local Dimensions",
                type: "vec3",
                vec3Type: "xyz",
                step: 0.01,
                decimals: 4,
                subLabels: [ "x", "y", "z" ],
                unit: "m",
                propertyID: "localDimensions",
                spaceMode: PROPERTY_SPACE_MODE.LOCAL,
            },
            {
                type: "buttons",
                buttons: [  { id: "copyDimensions", label: "Copy Dimensions", className: "secondary", onClick: copyDimensionsProperty },
                            { id: "pasteDimensions", label: "Paste Dimensions", className: "secondary", onClick: pasteDimensionsProperty } ],
                propertyID: "copyPasteDimensions"
            },
            {
                label: "Scale",
                type: "number-draggable",
                defaultValue: 100,
                unit: "%",
                buttons: [ { id: "rescale", label: "Rescale", className: "blue", onClick: rescaleDimensions }, 
                           { id: "reset", label: "Reset Dimensions", className: "red", onClick: resetToNaturalDimensions } ],
                propertyID: "scale",
            },
            {
                label: "Pivot",
                type: "vec3",
                vec3Type: "xyz",
                step: 0.001,
                decimals: 4,
                subLabels: [ "x", "y", "z" ],
                unit: "(ratio of dimension)",
                propertyID: "registrationPoint",
            },
            {
                label: "Align",
                type: "buttons",
                buttons: [ { id: "selection", label: "Selection to Grid", className: "black", onClick: moveSelectionToGrid },
                           { id: "all", label: "All to Grid", className: "black", onClick: moveAllToGrid } ],
                propertyID: "alignToGrid",
            }
        ]
    },
    {
        id: "behavior",
        label: "BEHAVIOR",
        properties: [
            {
                label: "Cloneable",
                type: "bool",
                propertyID: "cloneable",
            },
            {
                label: "Clone Lifetime",
                type: "number-draggable",
                min: -1,
                unit: "s",
                propertyID: "cloneLifetime",
                showPropertyRule: { "cloneable": "true" },
            },
            {
                label: "Clone Limit",
                type: "number-draggable",
                min: 0,
                propertyID: "cloneLimit",
                showPropertyRule: { "cloneable": "true" },
            },
            {
                label: "Clone Dynamic",
                type: "bool",
                propertyID: "cloneDynamic",
                showPropertyRule: { "cloneable": "true" },
            },
            {
                label: "Clone Avatar Entity",
                type: "bool",
                propertyID: "cloneAvatarEntity",
                showPropertyRule: { "cloneable": "true" },
            },
            {
                label: "Cast Shadows",
                type: "bool",
                propertyID: "canCastShadow",
            },
            {
                label: "Link",
                type: "string",
                propertyID: "href",
                placeholder: "URL",
            },
            {
                label: "Ignore Pick Intersection",
                type: "bool",
                propertyID: "ignorePickIntersection",
            },
            {
                label: "Lifetime",
                type: "number",
                unit: "s",
                propertyID: "lifetime",
            }
        ]
    },
    {
        id: "grabAndEquip",
        label: "GRAB AND EQUIP",
        properties: [
            {
                label: "Grabbable",
                type: "bool",
                propertyID: "grab.grabbable",
            },
            {
                label: "Follow Controller",
                type: "bool",
                propertyID: "grab.grabFollowsController",
                showPropertyRule: { "grab.grabbable": "true" },
            },
            {
                label: "Kinematic Grab",
                type: "bool",
                propertyID: "grab.grabKinematic",
                showPropertyRule: { "grab.grabbable": "true" },
            },
            {
                label: "Delegate To Parent",
                type: "bool",
                propertyID: "grab.grabDelegateToParent",
                showPropertyRule: { "grab.grabbable": "true" },
            },
            {
                label: "Triggerable",
                type: "bool",
                propertyID: "grab.triggerable",
            },
            {
                label: "Equippable",
                type: "bool",
                propertyID: "grab.equippable",
            },
            {
                label: "Left Position",
                type: "vec3",
                vec3Type: "xyz",
                subLabels: [ "x", "y", "z" ],
                step: 0.0025,
                decimals: 4,
                unit: "m",
                propertyID: "grab.equippableLeftPosition",
                showPropertyRule: { "grab.equippable": "true" },
            },
            {
                label: "Left Rotation",
                type: "vec3",
                vec3Type: "pyr",
                step: 0.1,
                decimals: 4,
                subLabels: [ "x", "y", "z" ],
                unit: "deg",
                propertyID: "grab.equippableLeftRotation",
                showPropertyRule: { "grab.equippable": "true" },
            },
            {
                label: "Right Position",
                type: "vec3",
                vec3Type: "xyz",
                subLabels: [ "x", "y", "z" ],
                step: 0.0025,
                decimals: 4,
                unit: "m",
                propertyID: "grab.equippableRightPosition",
                showPropertyRule: { "grab.equippable": "true" },
            },
            {
                label: "Right Rotation",
                type: "vec3",
                vec3Type: "pyr",
                step: 0.1,
                decimals: 4,
                subLabels: [ "x", "y", "z" ],
                unit: "deg",
                propertyID: "grab.equippableRightRotation",
                showPropertyRule: { "grab.equippable": "true" },
            },
            {
                label: "Indicator Model URL",
                type: "string",
                propertyID: "grab.equippableIndicatorURL",
                placeholder: "URL",
                showPropertyRule: { "grab.equippable": "true" },
            },
            {
                label: "Indicator Scale",
                type: "vec3",
                vec3Type: "xyz",
                subLabels: [ "x", "y", "z" ],
                step: 0.0025,
                decimals: 4,
                unit: "scale",
                propertyID: "grab.equippableIndicatorScale",
                showPropertyRule: { "grab.equippable": "true" },
            },
            {
                label: "Indicator Offset",
                type: "vec3",
                vec3Type: "xyz",
                subLabels: [ "x", "y", "z" ],
                step: 0.005,
                decimals: 4,
                unit: "m",
                propertyID: "grab.equippableIndicatorOffset",
                showPropertyRule: { "grab.equippable": "true" },
            },
        ]
    },
    {
        id: "scripts",
        label: "SCRIPTS",
        properties: [
            {
                label: "Script",
                type: "code",
                buttons: [ { id: "reload", label: "F", className: "glyph", onClick: reloadScripts } ],
                propertyID: "script",
                placeholder: "URL",
            },
            {
                label: "Server Script",
                type: "code",
                buttons: [ { id: "reload", label: "F", className: "glyph", onClick: reloadServerScripts } ],
                propertyID: "serverScripts",
                placeholder: "URL",
            },
            {
                label: "Server Script Status",
                type: "placeholder",
                indentedLabel: true,
                propertyID: "serverScriptStatus",
                selectionVisibility: PROPERTY_SELECTION_VISIBILITY.SINGLE_SELECTION,
            },
            {
                label: "User Data",
                type: "textarea",
                buttons: [ { id: "clear", label: "Clear User Data", className: "red", onClick: clearUserData }, 
                           { id: "edit", label: "Edit as JSON", className: "blue", onClick: newJSONEditor },
                           { id: "save", label: "Save User Data", className: "black", onClick: saveUserData } ],
                propertyID: "userData",
            }
        ]
    },
    {
        id: "collision",
        label: "COLLISION",
        properties: [
            {
                label: "Collides",
                type: "bool",
                inverse: true,
                propertyID: "collisionless",
            },
            {
                label: "Static Entities",
                type: "bool",
                propertyID: "collidesWithStatic",
                propertyName: "static", // actual subProperty name
                subPropertyOf: "collidesWith",
                showPropertyRule: { "collisionless": "false" },
            },
            {
                label: "Kinematic Entities",
                type: "bool",
                propertyID: "collidesWithKinematic",
                propertyName: "kinematic", // actual subProperty name
                subPropertyOf: "collidesWith",
                showPropertyRule: { "collisionless": "false" },
            },
            {
                label: "Dynamic Entities",
                type: "bool",
                propertyID: "collidesWithDynamic",
                propertyName: "dynamic", // actual subProperty name
                subPropertyOf: "collidesWith",
                showPropertyRule: { "collisionless": "false" },
            },
            {
                label: "My Avatar",
                type: "bool",
                propertyID: "collidesWithMyAvatar",
                propertyName: "myAvatar", // actual subProperty name
                subPropertyOf: "collidesWith",
                showPropertyRule: { "collisionless": "false" },
            },
            {
                label: "Other Avatars",
                type: "bool",
                propertyID: "collidesWithOtherAvatar",
                propertyName: "otherAvatar", // actual subProperty name
                subPropertyOf: "collidesWith",
                showPropertyRule: { "collisionless": "false" },
            },
            {
                label: "Collision Sound",
                type: "string",
                placeholder: "URL",
                propertyID: "collisionSoundURL",
                showPropertyRule: { "collisionless": "false" },
            },
            {
                label: "Dynamic",
                type: "bool",
                propertyID: "dynamic",
            }
        ]
    },
    {
        id: "physics",
        label: "PHYSICS",
        properties: [
            {
                label: "Linear Velocity",
                type: "vec3",
                vec3Type: "xyz",
                step: 0.01,
                decimals: 4,
                subLabels: [ "x", "y", "z" ],
                unit: "m/s",
                propertyID: "localVelocity",
            },
            {
                label: "Linear Damping",
                type: "number-draggable",
                min: 0,
                max: 1,
                step: 0.001,
                decimals: 4,
                propertyID: "damping",
            },
            {
                label: "Angular Velocity",
                type: "vec3",
                vec3Type: "pyr",
                multiplier: DEGREES_TO_RADIANS,
                decimals: 6,
                step: 1,
                subLabels: [ "x", "y", "z" ],
                unit: "deg/s",
                propertyID: "localAngularVelocity",
            },
            {
                label: "Angular Damping",
                type: "number-draggable",
                min: 0,
                max: 1,
                step: 0.001,
                decimals: 4,
                propertyID: "angularDamping",
            },
            {
                label: "Bounciness",
                type: "number-draggable",
                step: 0.001,
                decimals: 4,
                propertyID: "restitution",
            },
            {
                label: "Friction",
                type: "number-draggable",
                step: 0.01,
                decimals: 4,
                propertyID: "friction",
            },
            {
                label: "Density",
                type: "number-draggable",
                step: 1,
                decimals: 4,
                propertyID: "density",
            },
            {
                label: "Gravity",
                type: "vec3",
                vec3Type: "xyz",
                subLabels: [ "x", "y", "z" ],
                step: 0.1,
                decimals: 4,
                unit: "m/s<sup>2</sup>",
                propertyID: "gravity",
            }
        ]
    },
    {
        id: "children",
        label: "CHILD ENTITIES",
        properties: [
            {
                label: "Children",
                type: "childList",
                propertyID: "children",
            }
        ]
    },
];

const GROUPS_PER_TYPE = {
  None: [ 'base', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'collision', 'physics', 'children' ],
  Shape: [ 'base', 'shape', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'collision', 'physics', 'children' ],
  Text: [ 'base', 'text', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'collision', 'physics', 'children' ],
  Zone: [ 'base', 'zone', 'zone_key_light', 'zone_skybox', 'zone_ambient_light', 'zone_haze',
            'zone_bloom', 'zone_tonemapping', 'zone_ambient_occlusion', 'zone_avatar_priority',
            'zone_audio', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'physics', 'children' ],
  Model: [ 'base', 'model', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'collision', 'physics', 'children' ],
  Image: [ 'base', 'image', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'collision', 'physics', 'children' ],
  Web: [ 'base', 'web', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'collision', 'physics', 'children' ],
  Light: [ 'base', 'light', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'collision', 'physics', 'children' ],
  Material: [ 'base', 'material', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'physics', 'children' ],
  ParticleEffect: [ 'base', 'particles', 'particles_emit', 'particles_size', 'particles_color', 
                    'particles_behavior', 'particles_constraints', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'physics', 'children' ],
  ProceduralParticleEffect: [ 'base', 'particles_procedural', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'physics', 'children' ],
  PolyLine: [ 'base', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'collision', 'physics', 'children' ],
  PolyVox: [ 'base', 'polyvox', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'collision', 'physics', 'children' ],
  Grid: [ 'base', 'grid', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'physics', 'children' ],
  Sound: [ 'base', 'sound', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'physics', 'children' ],
  Multiple: [ 'base', 'spatial', 'behavior', 'grabAndEquip', 'scripts', 'collision', 'physics', 'children' ],
};

const EDITOR_TIMEOUT_DURATION = 1500;
const DEBOUNCE_TIMEOUT = 125;

const COLOR_MIN = 0;
const COLOR_MAX = 255;
const COLOR_STEP = 1;

const MATERIAL_PREFIX_STRING = "mat::";

const PENDING_SCRIPT_STATUS = "[ Fetching status ]";
const NOT_RUNNING_SCRIPT_STATUS = "Not running";
const ENTITY_SCRIPT_STATUS = {
    pending: "Pending",
    loading: "Loading",
    error_loading_script: "Error loading script", // eslint-disable-line camelcase
    error_running_script: "Error running script", // eslint-disable-line camelcase
    running: "Running",
    unloaded: "Unloaded"
};

const ENABLE_DISABLE_SELECTOR = "input, textarea, span, .dropdown dl, .color-picker";

const PROPERTY_NAME_DIVISION = {
    GROUP: 0,
    PROPERTY: 1,
    SUB_PROPERTY: 2,
};

const RECT_ELEMENTS = {
    X_NUMBER: 0,
    Y_NUMBER: 1,
    WIDTH_NUMBER: 2,
    HEIGHT_NUMBER: 3,
};

const VECTOR_ELEMENTS = {
    X_NUMBER: 0,
    Y_NUMBER: 1,
    Z_NUMBER: 2,
};

const COLOR_ELEMENTS = {
    COLOR_PICKER: 0,
    RED_NUMBER: 1,
    GREEN_NUMBER: 2,
    BLUE_NUMBER: 3,
};

const TEXTURE_ELEMENTS = {
    IMAGE: 0,
    TEXT_INPUT: 1,
};

const JSON_EDITOR_ROW_DIV_INDEX = 2;

let elGroups = {};
let properties = {};
let propertyRangeRequests = [];
let colorPickers = {};
let particlePropertyUpdates = {};
let selectedEntityIDs = new Set();
let currentSelections = [];
let createAppTooltip = new CreateAppTooltip();
let currentSpaceMode = PROPERTY_SPACE_MODE.LOCAL;
let zonesList = [];
let canViewAssetURLs = false;
let maSelectedId;
let skyboxColorForCopy;

function createElementFromHTML(htmlString) {
    let elTemplate = document.createElement('template');
    elTemplate.innerHTML = htmlString.trim();
    return elTemplate.content.firstChild;
}

function isFlagSet(value, flag) {
    return (value & flag) === flag;
}

/**
 * GENERAL PROPERTY/GROUP FUNCTIONS
 */

function getPropertyInputElement(propertyID) {
    let property = properties[propertyID];
    switch (property.data.type) {
        case 'string':
        case 'number':
        case 'bool':
        case 'dropdown':
        case 'textarea':
        case 'code':
        case 'texture':
            return property.elInput;
        case 'multipleZonesSelection':
            return property.elInput;
        case 'zonesCoefficient':
            return property.elInput;
        case 'arrayOfStrings':
            return property.elInput;
        case 'number-draggable':
            return property.elNumber.elInput;
        case 'rect':
            return {
                x: property.elNumberX.elInput,
                y: property.elNumberY.elInput,
                width: property.elNumberWidth.elInput,
                height: property.elNumberHeight.elInput
            };
        case 'vec3': 
        case 'vec2':
            return { x: property.elNumberX.elInput, y: property.elNumberY.elInput, z: property.elNumberZ.elInput };
        case 'color':
            return { red: property.elNumberR.elInput, green: property.elNumberG.elInput, blue: property.elNumberB.elInput };
        case 'vec3rgb':
            return { red: property.elNumberR.elInput, green: property.elNumberG.elInput, blue: property.elNumberB.elInput };            
        case 'icon':
            return property.elLabel;
        case 'dynamic-multiselect':
            return property.elDivOptions;
        default:
            return undefined;
    }
}

function enableChildren(el, selector) {
    let elSelectors = el.querySelectorAll(selector);
    for (let selectorIndex = 0; selectorIndex < elSelectors.length; ++selectorIndex) {
        elSelectors[selectorIndex].removeAttribute('disabled');
    }
}

function disableChildren(el, selector) {
    let elSelectors = el.querySelectorAll(selector);
    for (let selectorIndex = 0; selectorIndex < elSelectors.length; ++selectorIndex) {
        elSelectors[selectorIndex].setAttribute('disabled', 'disabled');
    }
}

function enableProperties() {
    enableChildren(document.getElementById("properties-list"), ENABLE_DISABLE_SELECTOR);
    enableChildren(document, ".colpick");
    enableAllMultipleZoneSelector();
}

function disableProperties() {
    disableChildren(document.getElementById("properties-list"), ENABLE_DISABLE_SELECTOR);
    disableChildren(document, ".colpick");
    for (let pickKey in colorPickers) {
        colorPickers[pickKey].colpickHide();
    }
    disableAllMultipleZoneSelector();
}

function showPropertyElement(propertyID, show) {
    setPropertyVisibility(properties[propertyID], show);
}

function setPropertyVisibility(property, visible) {
    property.elContainer.style.display = visible ? null : "none";
}

function setCopyPastePositionAndRotationAvailability (selectionLength, islocked) {
    if (selectionLength === 1) {
        $('#property-copyPastePosition-button-copyPosition').attr('disabled', false);
        $('#property-copyPasteRotation-button-copyRotation').attr('disabled', false);
        $('#property-copyPasteDimensions-button-copyDimensions').attr('disabled', false);
    } else {
        $('#property-copyPastePosition-button-copyPosition').attr('disabled', true);
        $('#property-copyPasteRotation-button-copyRotation').attr('disabled', true);
        $('#property-copyPasteDimensions-button-copyDimensions').attr('disabled', true);
    }
    
    if (selectionLength > 0 && !islocked) {
        $('#property-copyPastePosition-button-pastePosition').attr('disabled', false);
        $('#property-copyPasteRotation-button-pasteRotation').attr('disabled', false);
        $('#property-copyPasteDimensions-button-pasteDimensions').attr('disabled', false);
        if (selectionLength === 1) {
            $('#property-copyPasteRotation-button-setRotationToZero').attr('disabled', false);
        } else {
            $('#property-copyPasteRotation-button-setRotationToZero').attr('disabled', true);
        }
    } else {
        $('#property-copyPastePosition-button-pastePosition').attr('disabled', true);
        $('#property-copyPasteRotation-button-pasteRotation').attr('disabled', true);
        $('#property-copyPasteRotation-button-setRotationToZero').attr('disabled', true);
        $('#property-copyPasteDimensions-button-pasteDimensions').attr('disabled', true);
    }
}

function resetProperties() {
    for (let propertyID in properties) { 
        let property = properties[propertyID];
        let propertyData = property.data;
        
        switch (propertyData.type) {
            case 'number':
            case 'string': {
                property.elInput.classList.remove('multi-diff');
                if (propertyData.defaultValue !== undefined) {
                    property.elInput.value = propertyData.defaultValue;
                } else {
                    property.elInput.value = "";
                }
                break;
            }
            case 'bool': {
                property.elInput.classList.remove('multi-diff');
                property.elInput.checked = false;
                break;
            }
            case 'number-draggable': {
                if (propertyData.defaultValue !== undefined) {
                    property.elNumber.setValue(propertyData.defaultValue, false);
                } else { 
                    property.elNumber.setValue("", false);
                }
                break;
            }
            case 'rect': {
                property.elNumberX.setValue("", false);
                property.elNumberY.setValue("", false);
                property.elNumberWidth.setValue("", false);
                property.elNumberHeight.setValue("", false);
                break;
            }
            case 'vec3': 
            case 'vec2': {
                property.elNumberX.setValue("", false);
                property.elNumberY.setValue("", false);
                if (property.elNumberZ !== undefined) {
                    property.elNumberZ.setValue("", false);
                }
                break;
            }
            case 'color': {
                property.elColorPicker.style.backgroundColor = "rgb(" + 0 + "," + 0 + "," + 0 + ")";
                property.elNumberR.setValue("", false);
                property.elNumberG.setValue("", false);
                property.elNumberB.setValue("", false);
                break;
            }
            case 'vec3rgb': {
                property.elNumberR.setValue("", false);
                property.elNumberG.setValue("", false);
                property.elNumberB.setValue("", false);
                break;
            }            
            case 'dropdown': {
                property.elInput.classList.remove('multi-diff');
                property.elInput.value = "";
                setDropdownText(property.elInput);
                break;
            }
            case 'textarea':
            case 'code': {
                property.elInput.classList.remove('multi-diff');
                property.elInput.value = "";
                setTextareaScrolling(property.elInput);
                break;
            }
            case 'multipleZonesSelection': {
                property.elInput.classList.remove('multi-diff');
                property.elInput.value = "[]";
                setZonesSelectionData(property.elInput, false);
                break;
            }
            case 'arrayOfStrings': {
                property.elInput.classList.remove('multi-diff');
                property.elInput.value = "[]";
                setArrayOfStringsUi(property.elInput.id, false);
                break;
            }
            case 'zonesCoefficient': {
                property.elInput.classList.remove('multi-diff');
                property.elInput.value = "[]";
                setZonesCoefficientUi(property.elInput.id, false);
                break;
            }
            case 'childList': {
                setChildListData(property.elInput, undefined, "");
                break;
            }
            case 'icon': {
                property.elSpan.style.display = "none";
                break;
            }
            case 'texture': {
                property.elInput.classList.remove('multi-diff');
                property.elInput.value = "";
                property.elInput.imageLoad(property.elInput.value);
                break;
            }
            case 'dynamic-multiselect': {
                resetDynamicMultiselectProperty(property.elDivOptions);
                break;
            }
        }

        let showPropertyRules = properties[propertyID].showPropertyRules;
        if (showPropertyRules !== undefined) {
            for (let propertyToHide in showPropertyRules) {
                showPropertyElement(propertyToHide, false);
            }
        }
    }

    resetServerScriptStatus();
}

function resetServerScriptStatus() {
    let elServerScriptError = document.getElementById("property-serverScripts-error");
    let elServerScriptStatus = document.getElementById("property-serverScripts-status");
    elServerScriptError.parentElement.style.display = "none";
    elServerScriptStatus.innerText = NOT_RUNNING_SCRIPT_STATUS;
}

function showGroupsForType(type) {
    if (type === "Box" || type === "Sphere") {
        showGroupsForTypes(["Shape"]);
        showOnTheSamePage(["Shape"]);
        return;
    }
    if (type === "None") {
        showGroupsForTypes(["None"]);
        return;        
    }
    showGroupsForTypes([type]);
    showOnTheSamePage([type]);
}

function getGroupsForTypes(types) {
    return Object.keys(elGroups).filter((groupKey) => {
        return types.map(type => GROUPS_PER_TYPE[type].includes(groupKey)).every(function (hasGroup) {
            return hasGroup;
        });
    });
}

function showGroupsForTypes(types) {
    Object.entries(elGroups).forEach(([groupKey, elGroup]) => {
        if (types.map(type => GROUPS_PER_TYPE[type].includes(groupKey)).every(function (hasGroup) { return hasGroup; })) {
            elGroup.style.display = "none";
            if (types !== "None") {
                document.getElementById("tab-" + groupKey).style.display = "block";
            } else {
                document.getElementById("tab-" + groupKey).style.display = "none";
            }
        } else {
            elGroup.style.display = "none";
            document.getElementById("tab-" + groupKey).style.display = "none";
        }
    });
}

function getFirstSelectedID() {
    if (selectedEntityIDs.size === 0) {
        return null;
    }
    return selectedEntityIDs.values().next().value;
}

/**
 * Returns true when the user is currently dragging the numeric slider control of the property
 * @param propertyName - name of property
 * @returns {boolean} currentlyDragging
 */
function isCurrentlyDraggingProperty(propertyName) {
    return properties[propertyName] && properties[propertyName].dragging === true;
}

const SUPPORTED_FALLBACK_TYPES = ['number', 'number-draggable', 'rect', 'vec3', 'vec2', 'color', 'vec3rgb'];

function getMultiplePropertyValue(originalPropertyName) {
    // if this is a compound property name (i.e. animation.running)
    // then split it by . up to 3 times to find property value

    let propertyData = null;
    if (properties[originalPropertyName] !== undefined) {
        propertyData = properties[originalPropertyName].data;
    }

    let propertyValues = [];
    let splitPropertyName = originalPropertyName.split('.');
    if (splitPropertyName.length > 1) {
        let propertyGroupName = splitPropertyName[PROPERTY_NAME_DIVISION.GROUP];
        let propertyName = splitPropertyName[PROPERTY_NAME_DIVISION.PROPERTY];
        propertyValues = currentSelections.map(selection => {
            let groupProperties = selection.properties[propertyGroupName];
            if (groupProperties === undefined || groupProperties[propertyName] === undefined) {
                return undefined;
            }
            if (splitPropertyName.length === PROPERTY_NAME_DIVISION.SUB_PROPERTY + 1) {
                let subPropertyName = splitPropertyName[PROPERTY_NAME_DIVISION.SUB_PROPERTY];
                return groupProperties[propertyName][subPropertyName];
            } else {
                return groupProperties[propertyName];
            }
        });
    } else {
        propertyValues = currentSelections.map(selection => selection.properties[originalPropertyName]);
    }

    if (propertyData !== null && propertyData.fallbackProperty !== undefined &&
        SUPPORTED_FALLBACK_TYPES.includes(propertyData.type)) {

        let fallbackMultiValue = null;

        for (let i = 0; i < propertyValues.length; ++i) {
            let isPropertyNotNumber = false;
            let propertyValue = propertyValues[i];
            if (propertyValue === undefined) {
                continue;
            }
            switch (propertyData.type) {
                case 'number':
                case 'number-draggable':
                    isPropertyNotNumber = isNaN(propertyValue) || propertyValue === null;
                    break;
                case 'rect':
                case 'vec3':
                case 'vec2':
                    isPropertyNotNumber = isNaN(propertyValue.x) || propertyValue.x === null;
                    break;
                case 'color':
                    isPropertyNotNumber = isNaN(propertyValue.red) || propertyValue.red === null;
                    break;
                case 'vec3rgb':
                    isPropertyNotNumber = isNaN(propertyValue.red) || propertyValue.red === null;
                    break;
            }
            if (isPropertyNotNumber) {
                if (fallbackMultiValue === null) {
                    fallbackMultiValue = getMultiplePropertyValue(propertyData.fallbackProperty);
                }
                propertyValues[i] = fallbackMultiValue.values[i];
            }
        }
    }

    const firstValue = propertyValues[0];
    const isMultiDiffValue = !propertyValues.every((x) => deepEqual(firstValue, x));

    if (isMultiDiffValue) {
        return {
            value: undefined,
            values: propertyValues,
            isMultiDiffValue: true
        }
    }

    return {
        value: propertyValues[0],
        values: propertyValues,
        isMultiDiffValue: false
    };
}

/**
 * Retrieve more detailed info for differing Numeric MultiplePropertyValue
 * @param multiplePropertyValue - input multiplePropertyValue
 * @param propertyData
 * @returns {{keys: *[], propertyComponentDiff, averagePerPropertyComponent}}
 */
function getDetailedNumberMPVDiff(multiplePropertyValue, propertyData) {
    let detailedValues = {};
    // Fixed numbers can't be easily averaged since they're strings, so lets keep an array of unmodified numbers
    let unmodifiedValues = {};
    const DEFAULT_KEY = 0;
    let uniqueKeys = new Set([]);
    multiplePropertyValue.values.forEach(function(propertyValue) {
        if (typeof propertyValue === "object") {
            Object.entries(propertyValue).forEach(function([key, value]) {
                if (!uniqueKeys.has(key)) {
                    uniqueKeys.add(key);
                    detailedValues[key] = [];
                    unmodifiedValues[key] = [];
                }
                detailedValues[key].push(applyInputNumberPropertyModifiers(value, propertyData));
                unmodifiedValues[key].push(value);
            });
        } else {
            if (!uniqueKeys.has(DEFAULT_KEY)) {
                uniqueKeys.add(DEFAULT_KEY);
                detailedValues[DEFAULT_KEY] = [];
                unmodifiedValues[DEFAULT_KEY] = [];
            }
            detailedValues[DEFAULT_KEY].push(applyInputNumberPropertyModifiers(propertyValue, propertyData));
            unmodifiedValues[DEFAULT_KEY].push(propertyValue);
        }
    });
    let keys = [...uniqueKeys];

    let propertyComponentDiff = {};
    Object.entries(detailedValues).forEach(function([key, value]) {
        propertyComponentDiff[key] = [...new Set(value)].length > 1;
    });

    let averagePerPropertyComponent = {};
    Object.entries(unmodifiedValues).forEach(function([key, value]) {
        let average = value.reduce((a, b) => a + b) / value.length;
        averagePerPropertyComponent[key] = applyInputNumberPropertyModifiers(average, propertyData);
    });

    return {
        keys,
        propertyComponentDiff,
        averagePerPropertyComponent,
    };
}

function getDetailedSubPropertyMPVDiff(multiplePropertyValue, subPropertyName) {
    let isChecked = false;
    let checkedValues = multiplePropertyValue.values.map((value) => value.split(",").includes(subPropertyName));
    let isMultiDiff = !checkedValues.every(value => value === checkedValues[0]);
    if (!isMultiDiff) {
        isChecked = checkedValues[0];
    }
    return {
        isChecked,
        isMultiDiff
    }
}

function updateVisibleSpaceModeProperties() {
    for (let propertyID in properties) {
        if (properties.hasOwnProperty(propertyID)) {
            let property = properties[propertyID];
            let propertySpaceMode = property.spaceMode;
            let elProperty = properties[propertyID].elContainer;
            if (propertySpaceMode !== PROPERTY_SPACE_MODE.ALL && propertySpaceMode !== currentSpaceMode) {
                elProperty.classList.add('spacemode-hidden');
            } else {
                elProperty.classList.remove('spacemode-hidden');
            }
        }
    }
}

/**
 * PROPERTY UPDATE FUNCTIONS
 */

function createPropertyUpdateObject(originalPropertyName, propertyValue) {
    let propertyUpdate = {};
    // if this is a compound property name (i.e. animation.running) then split it by . up to 3 times
    let splitPropertyName = originalPropertyName.split('.');
    if (splitPropertyName.length > 1) {
        let propertyGroupName = splitPropertyName[PROPERTY_NAME_DIVISION.GROUP];
        let propertyName = splitPropertyName[PROPERTY_NAME_DIVISION.PROPERTY];
        propertyUpdate[propertyGroupName] = {};
        if (splitPropertyName.length === PROPERTY_NAME_DIVISION.SUB_PROPERTY + 1) {
            let subPropertyName = splitPropertyName[PROPERTY_NAME_DIVISION.SUB_PROPERTY];
            propertyUpdate[propertyGroupName][propertyName] = {};
            propertyUpdate[propertyGroupName][propertyName][subPropertyName] = propertyValue;
        } else {
            propertyUpdate[propertyGroupName][propertyName] = propertyValue;
        }
    } else {
        propertyUpdate[originalPropertyName] = propertyValue;
    }
    return propertyUpdate;
}

function updateProperty(originalPropertyName, propertyValue, isParticleProperty) {
    let propertyUpdate = createPropertyUpdateObject(originalPropertyName, propertyValue);

    // queue up particle property changes with the debounced sync to avoid  
    // causing particle emitting to reset excessively with each value change
    if (isParticleProperty) {
        Object.keys(propertyUpdate).forEach(function (propertyUpdateKey) {
            particlePropertyUpdates[propertyUpdateKey] = propertyUpdate[propertyUpdateKey];
        });
        particleSyncDebounce();
    } else {
        // only update the entity property value itself if in the middle of dragging
        // prevent undo command push, saving new property values, and property update
        // callback until drag is complete (additional update sent via dragEnd callback)
        let onlyUpdateEntity = isCurrentlyDraggingProperty(originalPropertyName);
        updateProperties(propertyUpdate, onlyUpdateEntity);
    }
}

let particleSyncDebounce = _.debounce(function () {
    updateProperties(particlePropertyUpdates);
    particlePropertyUpdates = {};
}, DEBOUNCE_TIMEOUT);

function updateProperties(propertiesToUpdate, onlyUpdateEntity) {
    if (onlyUpdateEntity === undefined) {
        onlyUpdateEntity = false;
    }
    EventBridge.emitWebEvent(JSON.stringify({
        ids: [...selectedEntityIDs],
        type: "update",
        properties: propertiesToUpdate,
        onlyUpdateEntities: onlyUpdateEntity
    }));
}

function updateMultiDiffProperties(propertiesMapToUpdate, onlyUpdateEntity) {
    if (onlyUpdateEntity === undefined) {
        onlyUpdateEntity = false;
    }
    EventBridge.emitWebEvent(JSON.stringify({
        type: "update",
        propertiesMap: propertiesMapToUpdate,
        onlyUpdateEntities: onlyUpdateEntity
    }));
}

function createEmitTextPropertyUpdateFunction(property) {
    return function() {
        // If we don't have canViewAssetURLs permissions, ignore clearing URLs
        if (!canViewAssetURLs && property.data.placeholder === "URL" && this.value === "") {
            return;
        }

        property.elInput.classList.remove('multi-diff');
        updateProperty(property.name, this.value, property.isParticleProperty);
    };
}

function createEmitCheckedPropertyUpdateFunction(property) {
    return function() {
        updateProperty(property.name, property.data.inverse ? !this.checked : this.checked, property.isParticleProperty);
    };
}

function createDragStartFunction(property) {
    return function() {
        property.dragging = true;
    };
}

function createDragEndFunction(property) {
    return function() {
        property.dragging = false;

        if (this.multiDiffModeEnabled) {
            let propertyMultiValue = getMultiplePropertyValue(property.name);
            let updateObjects = [];
            const selectedEntityIDsArray = [...selectedEntityIDs];

            for (let i = 0; i < selectedEntityIDsArray.length; ++i) {
                let entityID = selectedEntityIDsArray[i];
                updateObjects.push({
                    entityIDs: [entityID],
                    properties: createPropertyUpdateObject(property.name, propertyMultiValue.values[i]),
                });
            }

            // send a full updateMultiDiff post-dragging to count as an action in the undo stack
            updateMultiDiffProperties(updateObjects);
        } else {
            // send an additional update post-dragging to consider whole property change from dragStart to dragEnd to be 1 action
            this.valueChangeFunction();
        }
    };
}

function createEmitNumberPropertyUpdateFunction(property) {
    return function() {
        let value = parseFloat(applyOutputNumberPropertyModifiers(parseFloat(this.value), property.data));
        updateProperty(property.name, value, property.isParticleProperty);
    };
}

function createEmitNumberPropertyComponentUpdateFunction(property, propertyComponent) {
    return function() {
        let propertyMultiValue = getMultiplePropertyValue(property.name);
        let value = parseFloat(applyOutputNumberPropertyModifiers(parseFloat(this.value), property.data));

        if (propertyMultiValue.isMultiDiffValue) {
            let updateObjects = [];
            const selectedEntityIDsArray = [...selectedEntityIDs];

            for (let i = 0; i < selectedEntityIDsArray.length; ++i) {
                let entityID = selectedEntityIDsArray[i];

                let propertyObject = propertyMultiValue.values[i];
                propertyObject[propertyComponent] = value;

                let updateObject = createPropertyUpdateObject(property.name, propertyObject);
                updateObjects.push({
                    entityIDs: [entityID],
                    properties: updateObject,
                });

                mergeDeep(currentSelections[i].properties, updateObject);
            }

            // only update the entity property value itself if in the middle of dragging
            // prevent undo command push, saving new property values, and property update
            // callback until drag is complete (additional update sent via dragEnd callback)
            let onlyUpdateEntity = isCurrentlyDraggingProperty(property.name);
            updateMultiDiffProperties(updateObjects, onlyUpdateEntity);
        } else {
            let propertyValue = propertyMultiValue.value;
            propertyValue[propertyComponent] = value;
            updateProperty(property.name, propertyValue, property.isParticleProperty);
        }
    };
}

function createEmitColorPropertyUpdateFunction(property) {
    return function() {
        emitColorPropertyUpdate(property.name, property.elNumberR.elInput.value, property.elNumberG.elInput.value,
                                property.elNumberB.elInput.value, property.isParticleProperty);
    };
}

function emitColorPropertyUpdate(propertyName, red, green, blue, isParticleProperty) {
    let newValue = {
        red: red,
        green: green,
        blue: blue
    };
    updateProperty(propertyName, newValue, isParticleProperty);
}

function toggleBooleanCSV(inputCSV, property, enable) {
    let values = inputCSV.split(",");
    if (enable && !values.includes(property)) {
        values.push(property);
    } else if (!enable && values.includes(property)) {
        values = values.filter(value => value !== property);
    }
    return values.join(",");
}

function updateCheckedSubProperty(propertyName, propertyMultiValue, subPropertyElement, subPropertyString, isParticleProperty) {
    if (propertyMultiValue.isMultiDiffValue) {
        let updateObjects = [];
        const selectedEntityIDsArray = [...selectedEntityIDs];

        for (let i = 0; i < selectedEntityIDsArray.length; ++i) {
            let newValue = toggleBooleanCSV(propertyMultiValue.values[i], subPropertyString, subPropertyElement.checked);
            updateObjects.push({
                entityIDs: [selectedEntityIDsArray[i]],
                properties: createPropertyUpdateObject(propertyName, newValue),
            });
        }

        updateMultiDiffProperties(updateObjects);
    } else {
        updateProperty(propertyName, toggleBooleanCSV(propertyMultiValue.value, subPropertyString, subPropertyElement.checked),
            isParticleProperty);
    }
}

/**
 * PROPERTY ELEMENT CREATION FUNCTIONS
 */

function createStringProperty(property, elProperty) {
    let elementID = property.elementID;
    let propertyData = property.data;
    
    elProperty.className = "text";
    
    let elInput = createElementFromHTML(`
        <input id="${elementID}"
               type="text"
               ${propertyData.placeholder ? 'placeholder="' + ((propertyData.placeholder === "URL" && !canViewAssetURLs) ? "You don't have permission to view this URL" : propertyData.placeholder) + '"' : ''}
               ${propertyData.readOnly ? 'readonly' : ''}/>
        `);


    elInput.addEventListener('change', createEmitTextPropertyUpdateFunction(property));
    if (propertyData.onChange !== undefined) {
        elInput.addEventListener('change', propertyData.onChange);
    }


    let elMultiDiff = document.createElement('span');
    elMultiDiff.className = "multi-diff";

    elProperty.appendChild(elInput);
    elProperty.appendChild(elMultiDiff);
    
    if (propertyData.buttons !== undefined) {
        addButtons(elProperty, elementID, propertyData.buttons, false);
    }

    return elInput;
}

function createBoolProperty(property, elProperty) {   
    let propertyName = property.name;
    let elementID = property.elementID;
    let propertyData = property.data;

    elProperty.className = "checkbox";

    if (propertyData.glyph !== undefined) {
        let elSpan = document.createElement('span');
        elSpan.innerHTML = propertyData.glyph;
        elSpan.className = 'icon';
        elProperty.appendChild(elSpan);
    }
    
    let elInput = document.createElement('input');
    elInput.setAttribute("id", elementID);
    elInput.setAttribute("type", "checkbox");
    
    elProperty.appendChild(elInput);
    elProperty.appendChild(createElementFromHTML(`<label for=${elementID}>&nbsp;</label>`));
    
    let subPropertyOf = propertyData.subPropertyOf;
    if (subPropertyOf !== undefined) {
        elInput.addEventListener('change', function() {
            let subPropertyMultiValue = getMultiplePropertyValue(subPropertyOf);

            updateCheckedSubProperty(subPropertyOf,
                                     subPropertyMultiValue,
                                     elInput, propertyName, property.isParticleProperty);
        });
    } else {
        elInput.addEventListener('change', createEmitCheckedPropertyUpdateFunction(property));
    }
    
    return elInput;
}

function createNumberProperty(property, elProperty) {
    let elementID = property.elementID;
    let propertyData = property.data;

    elProperty.className = "text";

    let elInput = createElementFromHTML(`
        <input id="${elementID}"
               class='hide-spinner'
               type="number"
               ${propertyData.placeholder ? 'placeholder="' + propertyData.placeholder + '"' : ''}
               ${propertyData.readOnly ? 'readonly' : ''}/>
        `);

    if (propertyData.min !== undefined) {
        elInput.setAttribute("min", propertyData.min);
    }
    if (propertyData.max !== undefined) {
        elInput.setAttribute("max", propertyData.max);
    }
    if (propertyData.step !== undefined) {
        elInput.setAttribute("step", propertyData.step);
    }
    if (propertyData.defaultValue !== undefined) {
        elInput.value = propertyData.defaultValue;
    }

    elInput.addEventListener('change', createEmitNumberPropertyUpdateFunction(property));

    let elMultiDiff = document.createElement('span');
    elMultiDiff.className = "multi-diff";

    elProperty.appendChild(elInput);
    elProperty.appendChild(elMultiDiff);

    if (propertyData.buttons !== undefined) {
        addButtons(elProperty, elementID, propertyData.buttons, false);
    }

    return elInput;
}

function updateNumberMinMax(property) {
    let elInput = property.elInput;
    let min = property.data.min;
    let max = property.data.max;
    if (min !== undefined) {
        elInput.setAttribute("min", min);
    }
    if (max !== undefined) {
        elInput.setAttribute("max", max);
    }
}

/**
 *
 * @param {object} property - property update on step
 * @param {string} [propertyComponent] - propertyComponent to update on step (e.g. enter 'x' to just update position.x)
 * @returns {Function}
 */
function createMultiDiffStepFunction(property, propertyComponent) {
    return function(step, shouldAddToUndoHistory) {
        if (shouldAddToUndoHistory === undefined) {
            shouldAddToUndoHistory = false;
        }

        let propertyMultiValue = getMultiplePropertyValue(property.name);
        if (!propertyMultiValue.isMultiDiffValue) {
            console.log("setMultiDiffStepFunction is only supposed to be called in MultiDiff mode.");
            return;
        }

        let multiplier = property.data.multiplier !== undefined ? property.data.multiplier : 1;

        let applyDelta = step * multiplier;

        if (selectedEntityIDs.size !== propertyMultiValue.values.length) {
            console.log("selectedEntityIDs and propertyMultiValue got out of sync.");
            return;
        }
        let updateObjects = [];
        const selectedEntityIDsArray = [...selectedEntityIDs];

        for (let i = 0; i < selectedEntityIDsArray.length; ++i) {
            let entityID = selectedEntityIDsArray[i];

            let updatedValue;
            if (propertyComponent !== undefined) {
                let objectToUpdate = propertyMultiValue.values[i];
                objectToUpdate[propertyComponent] += applyDelta;
                updatedValue = objectToUpdate;
            } else {
                updatedValue = propertyMultiValue.values[i] + applyDelta;
            }
            let propertiesUpdate = createPropertyUpdateObject(property.name, updatedValue);
            updateObjects.push({
                entityIDs: [entityID],
                properties: propertiesUpdate
            });
            // We need to store these so that we can send a full update on the dragEnd
            mergeDeep(currentSelections[i].properties, propertiesUpdate);
        }

        updateMultiDiffProperties(updateObjects, !shouldAddToUndoHistory);
    }
}

function createNumberDraggableProperty(property, elProperty) { 
    let elementID = property.elementID;
    let propertyData = property.data;

    elProperty.className += " draggable-number-container";

    let dragStartFunction = createDragStartFunction(property);
    let dragEndFunction = createDragEndFunction(property);
    let elDraggableNumber = new DraggableNumber(propertyData.min, propertyData.max, propertyData.step,
                                                propertyData.decimals, dragStartFunction, dragEndFunction);

    let defaultValue = propertyData.defaultValue;
    if (defaultValue !== undefined) {
        elDraggableNumber.elInput.value = defaultValue;
    }

    let valueChangeFunction = createEmitNumberPropertyUpdateFunction(property);
    elDraggableNumber.setValueChangeFunction(valueChangeFunction);

    elDraggableNumber.setMultiDiffStepFunction(createMultiDiffStepFunction(property));
    
    elDraggableNumber.elInput.setAttribute("id", elementID);
    elProperty.appendChild(elDraggableNumber.elDiv);

    if (propertyData.buttons !== undefined) {
        addButtons(elDraggableNumber.elDiv, elementID, propertyData.buttons, false);
    }
    
    return elDraggableNumber;
}

function updateNumberDraggableMinMax(property) {
    let propertyData = property.data;
    property.elNumber.updateMinMax(propertyData.min, propertyData.max);
}

function createRectProperty(property, elProperty) {
    let propertyData = property.data;

    elProperty.className = "rect";

    let elXYRow = document.createElement('div');
    elXYRow.className = "rect-row fstuple";
    elProperty.appendChild(elXYRow);

    let elWidthHeightRow = document.createElement('div');
    elWidthHeightRow.className = "rect-row fstuple";
    elProperty.appendChild(elWidthHeightRow);


    let elNumberX = createTupleNumberInput(property, propertyData.subLabels[RECT_ELEMENTS.X_NUMBER]);
    let elNumberY = createTupleNumberInput(property, propertyData.subLabels[RECT_ELEMENTS.Y_NUMBER]);
    let elNumberWidth = createTupleNumberInput(property, propertyData.subLabels[RECT_ELEMENTS.WIDTH_NUMBER]);
    let elNumberHeight = createTupleNumberInput(property, propertyData.subLabels[RECT_ELEMENTS.HEIGHT_NUMBER]);

    elXYRow.appendChild(elNumberX.elDiv);
    elXYRow.appendChild(elNumberY.elDiv);
    elWidthHeightRow.appendChild(elNumberWidth.elDiv);
    elWidthHeightRow.appendChild(elNumberHeight.elDiv);

    elNumberX.setValueChangeFunction(createEmitNumberPropertyComponentUpdateFunction(property, 'x'));
    elNumberY.setValueChangeFunction(createEmitNumberPropertyComponentUpdateFunction(property, 'y'));
    elNumberWidth.setValueChangeFunction(createEmitNumberPropertyComponentUpdateFunction(property, 'width'));
    elNumberHeight.setValueChangeFunction(createEmitNumberPropertyComponentUpdateFunction(property, 'height'));

    elNumberX.setMultiDiffStepFunction(createMultiDiffStepFunction(property, 'x'));
    elNumberY.setMultiDiffStepFunction(createMultiDiffStepFunction(property, 'y'));
    elNumberX.setMultiDiffStepFunction(createMultiDiffStepFunction(property, 'width'));
    elNumberY.setMultiDiffStepFunction(createMultiDiffStepFunction(property, 'height'));

    let elResult = [];
    elResult[RECT_ELEMENTS.X_NUMBER] = elNumberX;
    elResult[RECT_ELEMENTS.Y_NUMBER] = elNumberY;
    elResult[RECT_ELEMENTS.WIDTH_NUMBER] = elNumberWidth;
    elResult[RECT_ELEMENTS.HEIGHT_NUMBER] = elNumberHeight;
    return elResult;
}

function updateRectMinMax(property) {
    let min = property.data.min;
    let max = property.data.max;
    property.elNumberX.updateMinMax(min, max);
    property.elNumberY.updateMinMax(min, max);
    property.elNumberWidth.updateMinMax(min, max);
    property.elNumberHeight.updateMinMax(min, max);
}

function createVec3Property(property, elProperty) {
    let propertyData = property.data;

    elProperty.className = propertyData.vec3Type + " fstuple";

    let elNumberX = createTupleNumberInput(property, propertyData.subLabels[VECTOR_ELEMENTS.X_NUMBER]);
    let elNumberY = createTupleNumberInput(property, propertyData.subLabels[VECTOR_ELEMENTS.Y_NUMBER]);
    let elNumberZ = createTupleNumberInput(property, propertyData.subLabels[VECTOR_ELEMENTS.Z_NUMBER]);
    elProperty.appendChild(elNumberX.elDiv);
    elProperty.appendChild(elNumberY.elDiv);
    elProperty.appendChild(elNumberZ.elDiv);

    elNumberX.setValueChangeFunction(createEmitNumberPropertyComponentUpdateFunction(property, 'x'));
    elNumberY.setValueChangeFunction(createEmitNumberPropertyComponentUpdateFunction(property, 'y'));
    elNumberZ.setValueChangeFunction(createEmitNumberPropertyComponentUpdateFunction(property, 'z'));

    elNumberX.setMultiDiffStepFunction(createMultiDiffStepFunction(property, 'x'));
    elNumberY.setMultiDiffStepFunction(createMultiDiffStepFunction(property, 'y'));
    elNumberZ.setMultiDiffStepFunction(createMultiDiffStepFunction(property, 'z'));

    let elResult = [];
    elResult[VECTOR_ELEMENTS.X_NUMBER] = elNumberX;
    elResult[VECTOR_ELEMENTS.Y_NUMBER] = elNumberY;
    elResult[VECTOR_ELEMENTS.Z_NUMBER] = elNumberZ;
    return elResult;
}

function createVec3rgbProperty(property, elProperty) {
    let propertyData = property.data;

    elProperty.className = propertyData.vec3Type + " fstuple";

    let elNumberR = createTupleNumberInput(property, propertyData.subLabels[VECTOR_ELEMENTS.X_NUMBER]);
    let elNumberG = createTupleNumberInput(property, propertyData.subLabels[VECTOR_ELEMENTS.Y_NUMBER]);
    let elNumberB = createTupleNumberInput(property, propertyData.subLabels[VECTOR_ELEMENTS.Z_NUMBER]);
    elProperty.appendChild(elNumberR.elDiv);
    elProperty.appendChild(elNumberG.elDiv);
    elProperty.appendChild(elNumberB.elDiv);

    elNumberR.setValueChangeFunction(createEmitNumberPropertyComponentUpdateFunction(property, 'red'));
    elNumberG.setValueChangeFunction(createEmitNumberPropertyComponentUpdateFunction(property, 'green'));
    elNumberB.setValueChangeFunction(createEmitNumberPropertyComponentUpdateFunction(property, 'blue'));

    elNumberR.setMultiDiffStepFunction(createMultiDiffStepFunction(property, 'red'));
    elNumberG.setMultiDiffStepFunction(createMultiDiffStepFunction(property, 'green'));
    elNumberB.setMultiDiffStepFunction(createMultiDiffStepFunction(property, 'blue'));

    let elResult = [];
    elResult[VECTOR_ELEMENTS.X_NUMBER] = elNumberR;
    elResult[VECTOR_ELEMENTS.Y_NUMBER] = elNumberG;
    elResult[VECTOR_ELEMENTS.Z_NUMBER] = elNumberB;
    return elResult;
}

function createVec2Property(property, elProperty) {
    let propertyData = property.data;
    
    elProperty.className = propertyData.vec2Type + " fstuple";

    let elTuple = document.createElement('div');
    elTuple.className = "tuple";
    
    elProperty.appendChild(elTuple);
    
    let elNumberX = createTupleNumberInput(property, propertyData.subLabels[VECTOR_ELEMENTS.X_NUMBER]);
    let elNumberY = createTupleNumberInput(property, propertyData.subLabels[VECTOR_ELEMENTS.Y_NUMBER]);
    elProperty.appendChild(elNumberX.elDiv);
    elProperty.appendChild(elNumberY.elDiv);

    elNumberX.setValueChangeFunction(createEmitNumberPropertyComponentUpdateFunction(property, 'x'));
    elNumberY.setValueChangeFunction(createEmitNumberPropertyComponentUpdateFunction(property, 'y'));

    elNumberX.setMultiDiffStepFunction(createMultiDiffStepFunction(property, 'x'));
    elNumberY.setMultiDiffStepFunction(createMultiDiffStepFunction(property, 'y'));
    
    let elResult = [];
    elResult[VECTOR_ELEMENTS.X_NUMBER] = elNumberX;
    elResult[VECTOR_ELEMENTS.Y_NUMBER] = elNumberY;
    return elResult;
}

function updateVectorMinMax(property) {
    let min = property.data.min;
    let max = property.data.max;
    if (property.elNumberX) {
        property.elNumberX.updateMinMax(min, max);
        property.elNumberY.updateMinMax(min, max);
        if (property.elNumberZ) {
            property.elNumberZ.updateMinMax(min, max);
        }
    } else if (property.elNumberR) {
        property.elNumberR.updateMinMax(min, max);
        property.elNumberG.updateMinMax(min, max);
        if (property.elNumberB) {
            property.elNumberB.updateMinMax(min, max);
        }
    }
}

function createColorProperty(property, elProperty) {
    let propertyName = property.name;
    let elementID = property.elementID;
    let propertyData = property.data;

    elProperty.className += " rgb fstuple";

    let elColorPicker = document.createElement('div');
    elColorPicker.className = "color-picker";
    elColorPicker.setAttribute("id", elementID);

    let elTuple = document.createElement('div');
    elTuple.className = "tuple";

    elProperty.appendChild(elColorPicker);
    elProperty.appendChild(elTuple);

    if (propertyData.min === undefined) {
        propertyData.min = COLOR_MIN;
    }
    if (propertyData.max === undefined) {
        propertyData.max = COLOR_MAX;
    }
    if (propertyData.step === undefined) {
        propertyData.step = COLOR_STEP;
    }

    let elNumberR = createTupleNumberInput(property, "red");
    let elNumberG = createTupleNumberInput(property, "green");
    let elNumberB = createTupleNumberInput(property, "blue");
    elTuple.appendChild(elNumberR.elDiv);
    elTuple.appendChild(elNumberG.elDiv);
    elTuple.appendChild(elNumberB.elDiv);

    let valueChangeFunction = createEmitColorPropertyUpdateFunction(property);
    elNumberR.setValueChangeFunction(valueChangeFunction);
    elNumberG.setValueChangeFunction(valueChangeFunction);
    elNumberB.setValueChangeFunction(valueChangeFunction);

    let colorPickerID = "#" + elementID;
    colorPickers[colorPickerID] = $(colorPickerID).colpick({
        colorScheme: 'dark',
        layout: 'rgbhex',
        color: '000000',
        submit: false, // We don't want to have a submission button
        onShow: function(colpick) {
            // The original color preview within the picker needs to be updated on show because
            // prior to the picker being shown we don't have access to the selections' starting color.
            colorPickers[colorPickerID].colpickSetColor({
                "r": elNumberR.elInput.value,
                "g": elNumberG.elInput.value,
                "b": elNumberB.elInput.value
            });

            // Set the color picker active after setting the color, otherwise an update will be sent on open.
            $(colorPickerID).attr('active', 'true');
        },
        onHide: function(colpick) {
            $(colorPickerID).attr('active', 'false');
        },
        onChange: function(hsb, hex, rgb, el) {
            $(el).css('background-color', '#' + hex);
            if ($(colorPickerID).attr('active') === 'true') {
                emitColorPropertyUpdate(propertyName, rgb.r, rgb.g, rgb.b);
            }
        }
    });

    let elResult = [];
    elResult[COLOR_ELEMENTS.COLOR_PICKER] = elColorPicker;
    elResult[COLOR_ELEMENTS.RED_NUMBER] = elNumberR;
    elResult[COLOR_ELEMENTS.GREEN_NUMBER] = elNumberG;
    elResult[COLOR_ELEMENTS.BLUE_NUMBER] = elNumberB;
    return elResult;
}

function createDropdownProperty(property, propertyID, elProperty) { 
    let elementID = property.elementID;
    let propertyData = property.data;
    
    elProperty.className = "dropdown";
                        
    let elInput = document.createElement('select');
    elInput.setAttribute("id", elementID);
    elInput.setAttribute("propertyID", propertyID);
    
    for (let optionKey in propertyData.options) {
        let option = document.createElement('option');
        option.value = optionKey;
        option.text = propertyData.options[optionKey];
        elInput.add(option);
    }

    elInput.addEventListener('change', createEmitTextPropertyUpdateFunction(property));

    elProperty.appendChild(elInput);

    return elInput;
}

function createTextareaProperty(property, elProperty) {
    let elementID = property.elementID;
    let propertyData = property.data;

    elProperty.className = "textarea";

    let elInput = document.createElement('textarea');
    elInput.setAttribute("id", elementID);
    if (propertyData.readOnly) {
        elInput.readOnly = true;
    }

    elInput.addEventListener('change', createEmitTextPropertyUpdateFunction(property));

    let elMultiDiff = document.createElement('span');
    elMultiDiff.className = "multi-diff";

    elProperty.appendChild(elInput);
    elProperty.appendChild(elMultiDiff);

    if (propertyData.buttons !== undefined) {
        addButtons(elProperty, elementID, propertyData.buttons, true);
    }

    return elInput;
}

function createCodeProperty(property, elProperty) {
    let elementID = property.elementID;
    let propertyData = property.data;

    elProperty.className = "textarea";

    let elInput = document.createElement('textarea');
    elInput.setAttribute("id", elementID);
    if (propertyData.readOnly) {
        elInput.readOnly = true;
    }

    elInput.addEventListener('change', createEmitTextPropertyUpdateFunction(property));
    elInput.addEventListener('keydown', function(event) {
        if (event.key === 'Tab') {
            event.preventDefault();
            const prevStart = this.selectionStart + 1;
            this.value = this.value.substring(0, this.selectionStart)
                       + "\t"
                       + this.value.substring(this.selectionEnd);
            this.selectionStart = prevStart;
            this.selectionEnd = prevStart;
        } else if (event.key === 'Escape') {
            event.preventDefault();
            this.blur();
        }
    });

    let elMultiDiff = document.createElement('span');
    elMultiDiff.className = "multi-diff";

    elProperty.appendChild(elInput);
    elProperty.appendChild(elMultiDiff);

    if (propertyData.buttons !== undefined) {
        addButtons(elProperty, elementID, propertyData.buttons, true);
    }

    return elInput;
}

function createIconProperty(property, elProperty) {
    let elementID = property.elementID;

    elProperty.className = "value";

    let elSpan = document.createElement('span');
    elSpan.setAttribute("id", elementID + "-icon");
    elSpan.className = 'icon';

    elProperty.appendChild(elSpan);

    return elSpan;
}

function createTextureProperty(property, elProperty) {
    let elementID = property.elementID;

    elProperty.className = "texture";

    let elDiv = document.createElement("div");
    let elImage = document.createElement("img");
    elDiv.className = "texture-image no-texture";
    elDiv.appendChild(elImage);

    let elInput = document.createElement('input');
    elInput.setAttribute("id", elementID);
    elInput.setAttribute("type", "text"); 

    let imageLoad = function(url) {
        elDiv.style.display = null;
        if (url.slice(0, 5).toLowerCase() === "atp:/" || url.slice(0, 9).toLowerCase() === "file:///~") {
            elImage.src = "";
            elImage.style.display = "none";
            elDiv.classList.remove("with-texture");
            elDiv.classList.remove("no-texture");
            elDiv.classList.add("no-preview");
        } else if (url.length > 0) {
            elDiv.classList.remove("no-texture");
            elDiv.classList.remove("no-preview");
            elDiv.classList.add("with-texture");
            elImage.src = url;
            elImage.style.display = "block";
        } else {
            elImage.src = "";
            elImage.style.display = "none";
            elDiv.classList.remove("with-texture");
            elDiv.classList.remove("no-preview");
            elDiv.classList.add("no-texture");
        }
    };
    elInput.imageLoad = imageLoad;
    elInput.setMultipleValues = function() {
        elDiv.style.display = "none";
    };
    elInput.addEventListener('change', createEmitTextPropertyUpdateFunction(property));
    elInput.addEventListener('change', function(ev) {
        imageLoad(ev.target.value);
    });

    elProperty.appendChild(elInput);
    let elMultiDiff = document.createElement('span');
    elMultiDiff.className = "multi-diff";
    elProperty.appendChild(elMultiDiff);
    elProperty.appendChild(elDiv);

    let elResult = [];
    elResult[TEXTURE_ELEMENTS.IMAGE] = elImage;
    elResult[TEXTURE_ELEMENTS.TEXT_INPUT] = elInput;
    return elResult;
}

function createButtonsProperty(property, elProperty) {
    let elementID = property.elementID;
    let propertyData = property.data;
    
    elProperty.className = "text";

    if (propertyData.buttons !== undefined) {
        addButtons(elProperty, elementID, propertyData.buttons, false);
    }

    return elProperty;
}

function createDynamicMultiselectProperty(property, elProperty) {
    let elementID = property.elementID;
    let propertyData = property.data;

    elProperty.className = "dynamic-multiselect";

    let elDivOptions = document.createElement('div');
    elDivOptions.setAttribute("id", elementID + "-options");
    elDivOptions.style = "overflow-y:scroll;max-height:160px;";

    let elDivButtons = document.createElement('div');
    elDivButtons.setAttribute("id", elDivOptions.getAttribute("id") + "-buttons");

    let elLabel = document.createElement('label');
    elLabel.innerText = "No Options";
    elDivOptions.appendChild(elLabel);

    let buttons = [ { id: "selectAll", label: "Select All", className: "black", onClick: selectAllMaterialTarget }, 
                    { id: "clearAll", label: "Clear All", className: "black", onClick: clearAllMaterialTarget } ];
    addButtons(elDivButtons, elementID, buttons, false);

    elProperty.appendChild(elDivOptions);
    elProperty.appendChild(elDivButtons);

    return elDivOptions;
}

function resetDynamicMultiselectProperty(elDivOptions) {
    let elInputs = elDivOptions.getElementsByTagName("input");
    while (elInputs.length > 0) {
        let elDivOption = elInputs[0].parentNode;
        elDivOption.parentNode.removeChild(elDivOption);
    }
    elDivOptions.firstChild.style.display = null; // show "No Options" text
    elDivOptions.parentNode.lastChild.style.display = "none"; // hide Select/Clear all buttons
}

function createTupleNumberInput(property, subLabel) {
    let propertyElementID = property.elementID;
    let propertyData = property.data;
    let elementID = propertyElementID + "-" + subLabel.toLowerCase();

    let elLabel = document.createElement('label');
    elLabel.className = "sublabel " + subLabel;
    elLabel.innerText = subLabel[0].toUpperCase() + subLabel.slice(1);
    elLabel.setAttribute("for", elementID);
    elLabel.style.visibility = "visible";

    let dragStartFunction = createDragStartFunction(property);
    let dragEndFunction = createDragEndFunction(property);
    let elDraggableNumber = new DraggableNumber(propertyData.min, propertyData.max, propertyData.step, 
                                                propertyData.decimals, dragStartFunction, dragEndFunction); 
    elDraggableNumber.elInput.setAttribute("id", elementID);
    elDraggableNumber.elDiv.className += " fstuple";
    elDraggableNumber.elDiv.insertBefore(elLabel, elDraggableNumber.elLeftArrow);

    return elDraggableNumber;
}

function addButtons(elProperty, propertyID, buttons, newRow) {
    let elDiv = document.createElement('div');
    elDiv.className = "row";

    buttons.forEach(function(button) {
        let elButton = document.createElement('input');
        elButton.className = button.className;
        elButton.setAttribute("type", "button");
        elButton.setAttribute("id", propertyID + "-button-" + button.id);
        elButton.setAttribute("value", button.label);
        elButton.addEventListener("click", button.onClick);
        if (newRow) {
            elDiv.appendChild(elButton);
        } else {
            elProperty.appendChild(elButton);
        }
    });
    
    if (newRow) {
        elProperty.appendChild(document.createElement('br'));
        elProperty.appendChild(elDiv);
    }
}

function createProperty(propertyData, propertyElementID, propertyName, propertyID, elProperty) {
    let property = {
        data: propertyData, 
        elementID: propertyElementID, 
        name: propertyName,
        elProperty: elProperty,
    };
    let propertyType = propertyData.type;

    switch (propertyType) {
        case 'string': {
            property.elInput = createStringProperty(property, elProperty);
            break;
        }
        case 'bool': {
            property.elInput = createBoolProperty(property, elProperty);
            break;
        }
        case 'number': {
            property.elInput = createNumberProperty(property, elProperty);
            break;
        }
        case 'number-draggable': {
            property.elNumber = createNumberDraggableProperty(property, elProperty);
            break;
        }
        case 'rect': {
            let elRect = createRectProperty(property, elProperty);
            property.elNumberX = elRect[RECT_ELEMENTS.X_NUMBER];
            property.elNumberY = elRect[RECT_ELEMENTS.Y_NUMBER];
            property.elNumberWidth = elRect[RECT_ELEMENTS.WIDTH_NUMBER];
            property.elNumberHeight = elRect[RECT_ELEMENTS.HEIGHT_NUMBER];
            break;
        }
        case 'vec3': {
            let elVec3 = createVec3Property(property, elProperty);  
            property.elNumberX = elVec3[VECTOR_ELEMENTS.X_NUMBER];
            property.elNumberY = elVec3[VECTOR_ELEMENTS.Y_NUMBER];
            property.elNumberZ = elVec3[VECTOR_ELEMENTS.Z_NUMBER];
            break;
        }
        case 'vec2': {
            let elVec2 = createVec2Property(property, elProperty);  
            property.elNumberX = elVec2[VECTOR_ELEMENTS.X_NUMBER];
            property.elNumberY = elVec2[VECTOR_ELEMENTS.Y_NUMBER];
            break;
        }
        case 'color': {
            let elColor = createColorProperty(property, elProperty);  
            property.elColorPicker = elColor[COLOR_ELEMENTS.COLOR_PICKER];
            property.elNumberR = elColor[COLOR_ELEMENTS.RED_NUMBER];
            property.elNumberG = elColor[COLOR_ELEMENTS.GREEN_NUMBER];
            property.elNumberB = elColor[COLOR_ELEMENTS.BLUE_NUMBER]; 
            break;
        }
        case 'vec3rgb': {
            let elVec3 = createVec3rgbProperty(property, elProperty);  
            property.elNumberR = elVec3[VECTOR_ELEMENTS.X_NUMBER];
            property.elNumberG = elVec3[VECTOR_ELEMENTS.Y_NUMBER];
            property.elNumberB = elVec3[VECTOR_ELEMENTS.Z_NUMBER];
            break;
        }        
        case 'dropdown': {
            property.elInput = createDropdownProperty(property, propertyID, elProperty);
            break;
        }
        case 'textarea': {
            property.elInput = createTextareaProperty(property, elProperty);
            break;
        }
        case 'code': {
            property.elInput = createCodeProperty(property, elProperty);
            break;
        }
        case 'multipleZonesSelection': {
            property.elInput = createZonesSelection(property, elProperty);
            break;
        }
        case 'arrayOfStrings': {
            property.elInput = createArrayOfStrings(property, elProperty);
            break;
        }
        case 'zonesCoefficient': {
            property.elInput = createZonesCoefficient(property, elProperty);
            break;
        }
        case 'childList': {
            property.elInput = createChildList(property, elProperty);
            break;
        }
        case 'icon': {
            property.elSpan = createIconProperty(property, elProperty);
            break;
        }
        case 'texture': {
            let elTexture = createTextureProperty(property, elProperty);
            property.elImage = elTexture[TEXTURE_ELEMENTS.IMAGE];
            property.elInput = elTexture[TEXTURE_ELEMENTS.TEXT_INPUT];
            break;
        }
        case 'buttons': {
            property.elProperty = createButtonsProperty(property, elProperty);
            break;
        }
        case 'dynamic-multiselect': {
            property.elDivOptions = createDynamicMultiselectProperty(property, elProperty);
            break;
        }
        case 'placeholder':
        case 'sub-header': {
            break;
        }
        default: {
            console.log("EntityProperties - Unknown property type " + 
                        propertyType + " set to property " + propertyID);
            break;
        }
    }

    return property;
}


/**
 * PROPERTY-SPECIFIC CALLBACKS
 */
 
function parentIDChanged() {
    if (currentSelections.length === 1 && currentSelections[0].properties.type === "Material") {
        requestMaterialTarget();
    }
}

function createPolyVoxPresetChangedFunction(property) {
    return function() {
        property.elInput.classList.remove('multi-diff');
        var xTextureURL = "";
        var yTextureURL = "";
        var zTextureURL = "";
        switch (parseInt(this.value)) {
            // Clear texture entries
            case 0:
                xTextureURL = "";
                yTextureURL = "";
                zTextureURL = "";
                break;
            // Grass + ground
            case 1:
                xTextureURL = "qrc:///serverless/Textures/ground_5-2K/2K-ground_5-diffuse.jpg";
                yTextureURL = "qrc:///serverless/Textures/ground_grass_gen_05.png";
                zTextureURL = "qrc:///serverless/Textures/ground_5-2K/2K-ground_5-diffuse.jpg";
                break;
            // Bricks
            case 2:
                xTextureURL = "qrc:///serverless/Textures/2K-wall_stone_2-diffuse_l.jpg";
                yTextureURL = "qrc:///serverless/Textures/2K-stone_floor_3-diffuse_l.jpg";
                zTextureURL = "qrc:///serverless/Textures/2K-wall_stone_2-diffuse_l.jpg";
                break;
            // Stone
            case 3:
                xTextureURL = "qrc:///serverless/Textures/wall_l.png";
                yTextureURL = "qrc:///serverless/Textures/floor_l.png";
                zTextureURL = "qrc:///serverless/Textures/wall_l.png";
                break;
            // Concrete
            case 4:
                xTextureURL = "qrc:///serverless/Textures/concrete_12-2K/2K-concrete_12-diffuse.jpg";
                yTextureURL = "qrc:///serverless/Textures/concrete_12-2K/2K-concrete_12-diffuse.jpg";
                zTextureURL = "qrc:///serverless/Textures/concrete_12-2K/2K-concrete_12-diffuse.jpg";
                break;
            // Rock
            case 5:
                xTextureURL = "qrc:///serverless/Textures/Rock026_2K-JPG/Rock026_2K_Color.jpg";
                yTextureURL = "qrc:///serverless/Textures/Rock026_2K-JPG/Rock026_2K_Color.jpg";
                zTextureURL = "qrc:///serverless/Textures/Rock026_2K-JPG/Rock026_2K_Color.jpg";
                break;
        }
        updateProperty("xTextureURL", xTextureURL, false);
        updateProperty("yTextureURL", yTextureURL, false);
        updateProperty("zTextureURL", zTextureURL, false);
    };
}

/**
 * BUTTON CALLBACKS
 */

function rescaleDimensions() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "action",
        action: "rescaleDimensions",
        percentage: parseFloat(document.getElementById("property-scale").value)
    }));
}

function moveSelectionToGrid() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "action",
        action: "moveSelectionToGrid"
    }));
}

function moveAllToGrid() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "action",
        action: "moveAllToGrid"
    }));
}

function resetToNaturalDimensions() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "action",
        action: "resetToNaturalDimensions"
    }));
}

function reloadScripts() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "action",
        action: "reloadClientScripts"
    }));
}

function reloadServerScripts() {
    // invalidate the current status (so that same-same updates can still be observed visually)
    document.getElementById("property-serverScripts-status").innerText = PENDING_SCRIPT_STATUS;
        EventBridge.emitWebEvent(JSON.stringify({
        type: "action",
        action: "reloadServerScripts"
    }));
}

function copySkyboxURLToAmbientURL() {
    let skyboxURL = getPropertyInputElement("skybox.url").value;
    getPropertyInputElement("ambientLight.ambientURL").value = skyboxURL;
    updateProperty("ambientLight.ambientURL", skyboxURL, false);
}

function copySkyboxColorToAmbientColor() {
    updateProperty("ambientLight.ambientColor", skyboxColorForCopy, false);
}

function copyPositionProperty() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "action",
        action: "copyPosition"
    }));
}

function pastePositionProperty() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "action",
        action: "pastePosition"
    }));
}

function copyRotationProperty() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "action",
        action: "copyRotation"
    }));
}

function pasteRotationProperty() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "action",
        action: "pasteRotation"
    }));
}
function setRotationToZeroProperty() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "action",
        action: "setRotationToZero"
    }));
}

function copyDimensionsProperty() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "action",
        action: "copyDimensions"
    }));
}

function pasteDimensionsProperty() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "action",
        action: "pasteDimensions"
    }));
}
/**
 * USER DATA FUNCTIONS
 */

function clearUserData() {
    let elUserData = getPropertyInputElement("userData");
    deleteJSONEditor();
    elUserData.value = "";
    showUserDataTextArea();
    showNewJSONEditorButton();
    hideSaveUserDataButton();
    updateProperty('userData', elUserData.value, false);
}

function newJSONEditor() {
    getPropertyInputElement("userData").classList.remove('multi-diff');
    deleteJSONEditor();
    createJSONEditor();
    let data = {};
    setEditorJSON(data);
    hideUserDataTextArea();
    hideNewJSONEditorButton();
    showSaveUserDataButton();
}

/**
 * @param {Set.<string>} [entityIDsToUpdate] Entity IDs to update userData for.
 */
function saveUserData(entityIDsToUpdate) {
    saveJSONUserData(true, entityIDsToUpdate);
}

function setJSONError(property, isError) {
    $("#property-"+ property + "-editor").toggleClass('error', isError);
    let $propertyUserDataEditorStatus = $("#property-"+ property + "-editorStatus");
    $propertyUserDataEditorStatus.css('display', isError ? 'block' : 'none');
    $propertyUserDataEditorStatus.text(isError ? 'Invalid JSON code - look for red X in your code' : '');
}

/**
 * @param {boolean} noUpdate - don't update the UI, but do send a property update.
 * @param {Set.<string>} [entityIDsToUpdate] - Entity IDs to update userData for.
 */
function setUserDataFromEditor(noUpdate, entityIDsToUpdate) {
    let errorFound = false;
    try {
        editor.get();
    } catch (e) {
        errorFound = true;
    }

    setJSONError('userData', errorFound);

    if (errorFound) {
        return;
    }

    let text = editor.getText();
    if (noUpdate) {
        EventBridge.emitWebEvent(
            JSON.stringify({
                ids: [...entityIDsToUpdate],
                type: "saveUserData",
                properties: {
                    userData: text
                }
            })
        );
    } else {
        updateProperty('userData', text, false);
    }
}

let editor = null;

function createJSONEditor() {
    let container = document.getElementById("property-userData-editor");
    let options = {
        search: false,
        mode: 'tree',
        modes: ['code', 'tree'],
        name: 'userData',
        onError: function(e) {
            alert('JSON editor:' + e);
        },
        onChange: function() {
            let currentJSONString = editor.getText();

            if (currentJSONString === '{"":""}') {
                return;
            }
            $('#property-userData-button-save').attr('disabled', false);
        }
    };
    editor = new JSONEditor(container, options);
}

function showSaveUserDataButton() {
    $('#property-userData-button-save').show();
}

function hideSaveUserDataButton() {
    $('#property-userData-button-save').hide();
}

function disableSaveUserDataButton() {
    $('#property-userData-button-save').attr('disabled', true);
}

function showNewJSONEditorButton() {
    $('#property-userData-button-edit').show();
}

function hideNewJSONEditorButton() {
    $('#property-userData-button-edit').hide();
}

function showUserDataTextArea() {
    $('#property-userData').show();
}

function hideUserDataTextArea() {
    $('#property-userData').hide();
}

function hideUserDataSaved() {
    $('#property-userData-saved').hide();
}

function setEditorJSON(json) {
    editor.set(json);
    if (editor.hasOwnProperty('expandAll')) {
        editor.expandAll();
    }
}

function deleteJSONEditor() {
    if (editor !== null) {
        setJSONError('userData', false);
        editor.destroy();
        editor = null;
    }
}

let savedJSONTimer = null;

/**
 * @param {boolean} noUpdate - don't update the UI, but do send a property update.
 * @param {Set.<string>} [entityIDsToUpdate] Entity IDs to update userData for
 */
function saveJSONUserData(noUpdate, entityIDsToUpdate) {
    setUserDataFromEditor(noUpdate, entityIDsToUpdate ? entityIDsToUpdate : selectedEntityIDs);
    $('#property-userData-saved').show();
    $('#property-userData-button-save').attr('disabled', true);
    if (savedJSONTimer !== null) {
        clearTimeout(savedJSONTimer);
    }
    savedJSONTimer = setTimeout(function() {
        hideUserDataSaved();
    }, EDITOR_TIMEOUT_DURATION);
}


/**
 * MATERIAL DATA FUNCTIONS
 */

function clearMaterialData() {
    let elMaterialData = getPropertyInputElement("materialData");
    deleteJSONMaterialEditor();
    elMaterialData.value = "";
    showMaterialDataTextArea();
    showNewJSONMaterialEditorButton();
    hideSaveMaterialDataButton();
    updateProperty('materialData', elMaterialData.value, false);
}

function newJSONMaterialEditor() {
    getPropertyInputElement("materialData").classList.remove('multi-diff');
    deleteJSONMaterialEditor();
    createJSONMaterialEditor();
    let data = {};
    setMaterialEditorJSON(data);
    hideMaterialDataTextArea();
    hideNewJSONMaterialEditorButton();
    showSaveMaterialDataButton();
}

function saveMaterialData() {
    saveJSONMaterialData(true);
}

function openMaterialAssistant() {
    if (materialEditor === null) {
        newJSONMaterialEditor();
    }
    loadDataInMaUi(materialEditor.getText());
    $('#uiMaterialAssistant').show();
    $('#properties-list').hide();
}

function closeMaterialAssistant() {
    $('#uiMaterialAssistant').hide();
    $('#properties-list').show();
}

/**
 * @param {boolean} noUpdate - don't update the UI, but do send a property update.
 * @param {Set.<string>} [entityIDsToUpdate] - Entity IDs to update materialData for.
 */
function setMaterialDataFromEditor(noUpdate, entityIDsToUpdate) {
    let errorFound = false;
    try {
        materialEditor.get();
    } catch (e) {
        errorFound = true;
    }

    setJSONError('materialData', errorFound);

    if (errorFound) {
        return;
    }
    let text = materialEditor.getText();
    if (noUpdate) {
        EventBridge.emitWebEvent(
            JSON.stringify({
                ids: [...entityIDsToUpdate],
                type: "saveMaterialData",
                properties: {
                    materialData: text
                }
            })
        );
    } else {
        updateProperty('materialData', text, false);
    }
    
    maGetMaterialDataAssistantAvailability(text);
}

let materialEditor = null;

function createJSONMaterialEditor() {
    let container = document.getElementById("property-materialData-editor");
    let options = {
        search: false,
        mode: 'tree',
        modes: ['code', 'tree'],
        name: 'materialData',
        onError: function(e) {
            alert('JSON editor:' + e);
        },
        onChange: function() {
            let currentJSONString = materialEditor.getText();

            if (currentJSONString === '{"":""}') {
                return;
            }
            $('#property-materialData-button-save').attr('disabled', false);
        }
    };
    materialEditor = new JSONEditor(container, options);
}

function showSaveMaterialDataButton() {
    $('#property-materialData-button-save').show();
}

function hideSaveMaterialDataButton() {
    $('#property-materialData-button-save').hide();
}

function disableSaveMaterialDataButton() {
    $('#property-materialData-button-save').attr('disabled', true);
}

function showNewJSONMaterialEditorButton() {
    $('#property-materialData-button-edit').show();
}

function hideNewJSONMaterialEditorButton() {
    $('#property-materialData-button-edit').hide();
}

function showMaterialDataTextArea() {
    $('#property-materialData').show();
}

function hideMaterialDataTextArea() {
    $('#property-materialData').hide();
}

function hideMaterialDataSaved() {
    $('#property-materialData-saved').hide();
}

function showMaterialAssistantButton() {
    $('#property-materialData-button-materialAssistant').show();
}

function hideMaterialAssistantButton() {
    $('#property-materialData-button-materialAssistant').hide();
}

function setMaterialEditorJSON(json) {
    materialEditor.set(json);
    if (materialEditor.hasOwnProperty('expandAll')) {
        materialEditor.expandAll();
    }
}

function deleteJSONMaterialEditor() {
    if (materialEditor !== null) {
        setJSONError('materialData', false);
        materialEditor.destroy();
        materialEditor = null;
    }
}

let savedMaterialJSONTimer = null;

/**
 * @param {boolean} noUpdate - don't update the UI, but do send a property update.
 * @param {Set.<string>} [entityIDsToUpdate] - Entity IDs to update materialData for.
 */
function saveJSONMaterialData(noUpdate, entityIDsToUpdate) {
    setMaterialDataFromEditor(noUpdate, entityIDsToUpdate ? entityIDsToUpdate : selectedEntityIDs);
    $('#property-materialData-saved').show();
    $('#property-materialData-button-save').attr('disabled', true);
    if (savedMaterialJSONTimer !== null) {
        clearTimeout(savedMaterialJSONTimer);
    }
    savedMaterialJSONTimer = setTimeout(function() {
        hideMaterialDataSaved();
    }, EDITOR_TIMEOUT_DURATION);
}


/**
 * PROCEDURAL PARTICLE DATA FUNCTIONS
 */

function clearParticleUpdateData() {
    let elParticleUpdateData = getPropertyInputElement("particleUpdateData");
    deleteJSONParticleUpdateEditor();
    elParticleUpdateData.value = "";
    showParticleUpdateDataTextArea();
    showNewJSONParticleUpdateEditorButton();
    hideSaveParticleUpdateDataButton();
    updateProperty('particleUpdateData', elParticleUpdateData.value, false);
}

function newJSONParticleUpdateEditor() {
    getPropertyInputElement("particleUpdateData").classList.remove('multi-diff');
    deleteJSONParticleUpdateEditor();
    createJSONParticleUpdateEditor();
    let data = {};
    setParticleUpdateEditorJSON(data);
    hideParticleUpdateDataTextArea();
    hideNewJSONParticleUpdateEditorButton();
    showSaveParticleUpdateDataButton();
}

/**
 * @param {Set.<string>} [entityIDsToUpdate] Entity IDs to update particleUpdateData for.
 */
function saveParticleUpdateData(entityIDsToUpdate) {
    saveJSONParticleUpdateData(true, entityIDsToUpdate);
}

function setJSONError(property, isError) {
    $("#property-"+ property + "-editor").toggleClass('error', isError);
    let $propertyParticleUpdateDataEditorStatus = $("#property-"+ property + "-editorStatus");
    $propertyParticleUpdateDataEditorStatus.css('display', isError ? 'block' : 'none');
    $propertyParticleUpdateDataEditorStatus.text(isError ? 'Invalid JSON code - look for red X in your code' : '');
}

/**
 * @param {boolean} noUpdate - don't update the UI, but do send a property update.
 * @param {Set.<string>} [entityIDsToUpdate] - Entity IDs to update particleUpdateData for.
 */
function setParticleUpdateDataFromEditor(noUpdate, entityIDsToUpdate) {
    let errorFound = false;
    try {
        particleUpdateEditor.get();
    } catch (e) {
        errorFound = true;
    }

    setJSONError('particleUpdateData', errorFound);

    if (errorFound) {
        return;
    }

    let text = particleUpdateEditor.getText();
    if (noUpdate) {
        EventBridge.emitWebEvent(
            JSON.stringify({
                ids: [...entityIDsToUpdate],
                type: "saveParticleUpdateData",
                properties: {
                    particleUpdateData: text
                }
            })
        );
    } else {
        updateProperty('particleUpdateData', text, false);
    }
}

let particleUpdateEditor = null;

function createJSONParticleUpdateEditor() {
    let container = document.getElementById("property-particleUpdateData-editor");
    let options = {
        search: false,
        mode: 'tree',
        modes: ['code', 'tree'],
        name: 'particleUpdateData',
        onError: function(e) {
            alert('JSON editor:' + e);
        },
        onChange: function() {
            let currentJSONString = particleUpdateEditor.getText();

            if (currentJSONString === '{"":""}') {
                return;
            }
            $('#property-particleUpdateData-button-save').attr('disabled', false);
        }
    };
    particleUpdateEditor = new JSONEditor(container, options);
}

function showSaveParticleUpdateDataButton() {
    $('#property-particleUpdateData-button-save').show();
}

function hideSaveParticleUpdateDataButton() {
    $('#property-particleUpdateData-button-save').hide();
}

function disableSaveParticleUpdateDataButton() {
    $('#property-particleUpdateData-button-save').attr('disabled', true);
}

function showNewJSONParticleUpdateEditorButton() {
    $('#property-particleUpdateData-button-edit').show();
}

function hideNewJSONParticleUpdateEditorButton() {
    $('#property-particleUpdateData-button-edit').hide();
}

function showParticleUpdateDataTextArea() {
    $('#property-particleUpdateData').show();
}

function hideParticleUpdateDataTextArea() {
    $('#property-particleUpdateData').hide();
}

function hideParticleUpdateDataSaved() {
    $('#property-particleUpdateData-saved').hide();
}

function setParticleUpdateEditorJSON(json) {
    particleUpdateEditor.set(json);
    if (particleUpdateEditor.hasOwnProperty('expandAll')) {
        particleUpdateEditor.expandAll();
    }
}

function deleteJSONParticleUpdateEditor() {
    if (particleUpdateEditor !== null) {
        setJSONError('particleUpdateData', false);
        particleUpdateEditor.destroy();
        particleUpdateEditor = null;
    }
}

let savedParticleUpdateJSONTimer = null;

/**
 * @param {boolean} noUpdate - don't update the UI, but do send a property update.
 * @param {Set.<string>} [entityIDsToUpdate] Entity IDs to update particleUpdateData for
 */
function saveJSONParticleUpdateData(noUpdate, entityIDsToUpdate) {
    setParticleUpdateDataFromEditor(noUpdate, entityIDsToUpdate ? entityIDsToUpdate : selectedEntityIDs);
    $('#property-particleUpdateData-saved').show();
    $('#property-particleUpdateData-button-save').attr('disabled', true);
    if (savedJSONTimer !== null) {
        clearTimeout(savedJSONTimer);
    }
    savedJSONTimer = setTimeout(function() {
        hideParticleUpdateDataSaved();
    }, EDITOR_TIMEOUT_DURATION);
}

function clearParticleRenderData() {
    let elParticleRenderData = getPropertyInputElement("particleRenderData");
    deleteJSONParticleRenderEditor();
    elParticleRenderData.value = "";
    showParticleRenderDataTextArea();
    showNewJSONParticleRenderEditorButton();
    hideSaveParticleRenderDataButton();
    updateProperty('particleRenderData', elParticleRenderData.value, false);
}

function newJSONParticleRenderEditor() {
    getPropertyInputElement("particleRenderData").classList.remove('multi-diff');
    deleteJSONParticleRenderEditor();
    createJSONParticleRenderEditor();
    let data = {};
    setParticleRenderEditorJSON(data);
    hideParticleRenderDataTextArea();
    hideNewJSONParticleRenderEditorButton();
    showSaveParticleRenderDataButton();
}

/**
 * @param {Set.<string>} [entityIDsToUpdate] Entity IDs to update particleRenderData for.
 */
function saveParticleRenderData(entityIDsToUpdate) {
    saveJSONParticleRenderData(true, entityIDsToUpdate);
}

function setJSONError(property, isError) {
    $("#property-"+ property + "-editor").toggleClass('error', isError);
    let $propertyParticleRenderDataEditorStatus = $("#property-"+ property + "-editorStatus");
    $propertyParticleRenderDataEditorStatus.css('display', isError ? 'block' : 'none');
    $propertyParticleRenderDataEditorStatus.text(isError ? 'Invalid JSON code - look for red X in your code' : '');
}

/**
 * @param {boolean} noUpdate - don't update the UI, but do send a property update.
 * @param {Set.<string>} [entityIDsToUpdate] - Entity IDs to update particleRenderData for.
 */
function setParticleRenderDataFromEditor(noUpdate, entityIDsToUpdate) {
    let errorFound = false;
    try {
        particleRenderEditor.get();
    } catch (e) {
        errorFound = true;
    }

    setJSONError('particleRenderData', errorFound);

    if (errorFound) {
        return;
    }

    let text = particleRenderEditor.getText();
    if (noUpdate) {
        EventBridge.emitWebEvent(
            JSON.stringify({
                ids: [...entityIDsToUpdate],
                type: "saveParticleRenderData",
                properties: {
                    particleRenderData: text
                }
            })
        );
    } else {
        updateProperty('particleRenderData', text, false);
    }
}

let particleRenderEditor = null;

function createJSONParticleRenderEditor() {
    let container = document.getElementById("property-particleRenderData-editor");
    let options = {
        search: false,
        mode: 'tree',
        modes: ['code', 'tree'],
        name: 'particleRenderData',
        onError: function(e) {
            alert('JSON editor:' + e);
        },
        onChange: function() {
            let currentJSONString = particleRenderEditor.getText();

            if (currentJSONString === '{"":""}') {
                return;
            }
            $('#property-particleRenderData-button-save').attr('disabled', false);
        }
    };
    particleRenderEditor = new JSONEditor(container, options);
}

function showSaveParticleRenderDataButton() {
    $('#property-particleRenderData-button-save').show();
}

function hideSaveParticleRenderDataButton() {
    $('#property-particleRenderData-button-save').hide();
}

function disableSaveParticleRenderDataButton() {
    $('#property-particleRenderData-button-save').attr('disabled', true);
}

function showNewJSONParticleRenderEditorButton() {
    $('#property-particleRenderData-button-edit').show();
}

function hideNewJSONParticleRenderEditorButton() {
    $('#property-particleRenderData-button-edit').hide();
}

function showParticleRenderDataTextArea() {
    $('#property-particleRenderData').show();
}

function hideParticleRenderDataTextArea() {
    $('#property-particleRenderData').hide();
}

function hideParticleRenderDataSaved() {
    $('#property-particleRenderData-saved').hide();
}

function setParticleRenderEditorJSON(json) {
    particleRenderEditor.set(json);
    if (particleRenderEditor.hasOwnProperty('expandAll')) {
        particleRenderEditor.expandAll();
    }
}

function deleteJSONParticleRenderEditor() {
    if (particleRenderEditor !== null) {
        setJSONError('particleRenderData', false);
        particleRenderEditor.destroy();
        particleRenderEditor = null;
    }
}

let savedParticleRenderJSONTimer = null;

/**
 * @param {boolean} noUpdate - don't update the UI, but do send a property update.
 * @param {Set.<string>} [entityIDsToUpdate] Entity IDs to update particleRenderData for
 */
function saveJSONParticleRenderData(noUpdate, entityIDsToUpdate) {
    setParticleRenderDataFromEditor(noUpdate, entityIDsToUpdate ? entityIDsToUpdate : selectedEntityIDs);
    $('#property-particleRenderData-saved').show();
    $('#property-particleRenderData-button-save').attr('disabled', true);
    if (savedJSONTimer !== null) {
        clearTimeout(savedJSONTimer);
    }
    savedJSONTimer = setTimeout(function() {
        hideParticleRenderDataSaved();
    }, EDITOR_TIMEOUT_DURATION);
}

function bindAllNonJSONEditorElements() {
    let inputs = $('input');
    let i;
    for (i = 0; i < inputs.length; ++i) {
        let input = inputs[i];
        let field = $(input);
        // TODO FIXME: (JSHint) Functions declared within loops referencing 
        //             an outer scoped variable may lead to confusing semantics.
        field.on('focus', function(e) {
            if (e.target.id === "property-userData-button-edit" || e.target.id === "property-userData-button-clear" || 
                e.target.id === "property-materialData-button-edit" || e.target.id === "property-materialData-button-clear" ||
                e.target.id === "property-particleUpdateData-button-edit" || e.target.id === "property-particleUpdateData-button-clear" ||
                e.target.id === "property-particleRenderData-button-edit" || e.target.id === "property-particleRenderData-button-clear") {
                return;
            }
            if ($('#property-userData-editor').css('height') !== "0px") {
                saveUserData();
            }
            if ($('#property-materialData-editor').css('height') !== "0px") {
                saveMaterialData();
            }
            if ($('#property-particleUpdateData-editor').css('height') !== "0px") {
                saveParticleUpdateData();
            }
            if ($('#property-particleRenderData-editor').css('height') !== "0px") {
                saveParticleRenderData();
            }
        });
    }
}


/**
 * DROPDOWN FUNCTIONS
 */

function setDropdownText(dropdown) {
    let lis = dropdown.parentNode.getElementsByTagName("li");
    let text = "";
    for (let i = 0; i < lis.length; ++i) {
        if (String(lis[i].getAttribute("value")) === String(dropdown.value)) {
            text = lis[i].textContent;
        }
    }
    dropdown.firstChild.textContent = text;
}

function toggleDropdown(event) {
    let element = event.target;
    if (element.nodeName !== "DT") {
        element = element.parentNode;
    }
    element = element.parentNode;
    let isDropped = element.getAttribute("dropped");
    element.setAttribute("dropped", isDropped !== "true" ? "true" : "false");
}

function closeAllDropdowns() {
    let elDropdowns = document.querySelectorAll("div.dropdown > dl");
    for (let i = 0; i < elDropdowns.length; ++i) {
        elDropdowns[i].setAttribute('dropped', 'false');
    }
}

function setDropdownValue(event) {
    let dt = event.target.parentNode.parentNode.previousSibling.previousSibling;
    dt.value = event.target.getAttribute("value");
    dt.firstChild.textContent = event.target.textContent;

    dt.parentNode.setAttribute("dropped", "false");

    let evt = document.createEvent("HTMLEvents");
    evt.initEvent("change", true, true);
    dt.dispatchEvent(evt);
}


/**
 * TEXTAREA FUNCTIONS
 */

function setTextareaScrolling(element) {
    let isScrolling = element.scrollHeight > element.offsetHeight;
    element.setAttribute("scrolling", isScrolling ? "true" : "false");
}

/**
 * ZONE SELECTOR FUNCTIONS
 */

function enableAllMultipleZoneSelector() {
    let allMultiZoneSelectors = document.querySelectorAll(".hiddenMultiZonesSelection");
    let i, propId;
    for (i = 0; i < allMultiZoneSelectors.length; i++) {
        propId = allMultiZoneSelectors[i].id;
        displaySelectedZones(propId, true);
    }
} 

function disableAllMultipleZoneSelector() {
    let allMultiZoneSelectors = document.querySelectorAll(".hiddenMultiZonesSelection");
    let i, propId;
    for (i = 0; i < allMultiZoneSelectors.length; i++) {
        propId = allMultiZoneSelectors[i].id;
        displaySelectedZones(propId, false);
    }
} 

function requestZoneList() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "zoneListRequest"
    }));
}

function addZoneToZonesSelection(propertyId, id) {
    let hiddenField = document.getElementById(propertyId);
    if (JSON.stringify(hiddenField.value) === '"undefined"') {
        hiddenField.value = "[]";
    }
    let zonesCoefficientID = document.getElementById(propertyId).getAttribute('zonesCoefficientID');
    let selectedZones = JSON.parse(hiddenField.value);
    if (id === "ALL") {
        for (let i = 0; i < zonesList.length; i++) {
            if (!selectedZones.includes(zonesList[i].id)) {
                selectedZones.push(zonesList[i].id);
                if (zonesCoefficientID !== "") {
                    addElementToZonesCoefficient("property-" + zonesCoefficientID.replace(".", "-"));
                }
            }
        }        
    } else {
        if (!selectedZones.includes(id)) {
            selectedZones.push(id);
            if (zonesCoefficientID !== "") {
                addElementToZonesCoefficient("property-" + zonesCoefficientID.replace(".", "-"));
            }
        }
    }
    hiddenField.value = JSON.stringify(selectedZones);
    displaySelectedZones(propertyId, true);
    let propertyName = propertyId.replace("property-", "");
    propertyName = propertyName.replace("-", ".");
    updateProperty(propertyName, selectedZones, false);
    document.getElementById("zones-select-selector-list-panel-" + propertyId).style.display = "none";
}

function removeZoneFromZonesSelection(propertyId, zoneId) {
    let zonesCoefficientID = document.getElementById(propertyId).getAttribute('zonesCoefficientID');
    let hiddenField = document.getElementById(propertyId);
    if (JSON.stringify(hiddenField.value) === '"undefined"') {
        hiddenField.value = "[]";
    }
    let selectedZones = JSON.parse(hiddenField.value);
    let index = selectedZones.indexOf(zoneId);
    if (index > -1) {
        selectedZones.splice(index, 1);
        if (zonesCoefficientID !== "") {
            removeElementFromZonesCoefficient("property-" + zonesCoefficientID.replace(".", "-"), index);
        }
    }
    hiddenField.value = JSON.stringify(selectedZones);
    displaySelectedZones(propertyId, true);
    let propertyName = propertyId.replace("property-", "");
    propertyName = propertyName.replace("-", ".");
    updateProperty(propertyName, selectedZones, false);
}

function displaySelectedZones(propertyId, isEditable) {
    let i,j, name, listedZoneInner, hiddenData, isMultiple;
    hiddenData = document.getElementById(propertyId).value;
    if (JSON.stringify(hiddenData) === '"undefined"') {
        isMultiple = true;
        hiddenData = "[]";
    } else {
        isMultiple = false;  
    }
    let selectedZones = JSON.parse(hiddenData);
    listedZoneInner = "<table>";
    if (selectedZones.length === 0) {
        if (!isMultiple) {
            listedZoneInner += "<tr><td class='zoneItem'>&nbsp;</td><td>&nbsp;</td></tr>";
        } else {
            listedZoneInner += "<tr><td class='zoneItem'>[ WARNING: Any changes will apply to all. ]</td><td>&nbsp;</td></tr>";
        }
    } else {
        for (i = 0; i < selectedZones.length; i++) {
            name = getZoneName(selectedZones[i]);
            if (isEditable) {
                listedZoneInner += "<tr><td class='zoneItem'>" + name + "</td><td><a href='#' onClick='removeZoneFromZonesSelection(" + '"' + propertyId + '"' + ", " + '"' + selectedZones[i] + '"' + ");' >";
                listedZoneInner += "<img src='../../../html/css/img/remove_icon.png'></a></td></tr>";
            } else {
                listedZoneInner += "<tr><td class='zoneItem'>" + name + "</td><td>&nbsp;</td></tr>";
            }
        }
    }
    listedZoneInner += "</table>";
    document.getElementById("selected-zones-" + propertyId).innerHTML = listedZoneInner; 
    if (isEditable) {
        document.getElementById("multiZoneSelTools-" + propertyId).style.display = "block";
    } else {
        document.getElementById("multiZoneSelTools-" + propertyId).style.display = "none";
    }
}

function createZonesSelection(property, elProperty) {
    let propertyData = property.data;
    let elementID = property.elementID;
    requestZoneList();
    elProperty.className = "multipleZonesSelection";
    let elInput = document.createElement('input');
    elInput.setAttribute("id", elementID);
    elInput.setAttribute("type", "hidden");
    elInput.setAttribute("zonesCoefficientID", propertyData.zonesCoefficientID ?? "");
    elInput.className = "hiddenMultiZonesSelection";

    let elZonesSelector = document.createElement('div');
    elZonesSelector.setAttribute("id", "zones-selector-" + elementID);

    let elMultiDiff = document.createElement('span');
    elMultiDiff.className = "multi-diff";

    elProperty.appendChild(elInput);
    elProperty.appendChild(elZonesSelector);
    elProperty.appendChild(elMultiDiff);

    return elInput;
}

function setZonesSelectionData(element, isEditable) {
    let zoneSelectorContainer = document.getElementById("zones-selector-" + element.id);
    let zoneSelector = "<div class='multiZoneSelToolbar' id='multiZoneSelTools-" + element.id + "'>";
    zoneSelector += "<input type='button' value = 'Add a Zone' id='zones-select-add-" + element.id + "' onClick='document.getElementById(";
    zoneSelector += '"' + "zones-select-selector-list-panel-" + element.id + '"' + ").style.display = " + '"' + "block" + '"' + ";'>";
    zoneSelector += "<div class = 'zoneSelectorListPanel' id='zones-select-selector-list-panel-" + element.id + "'>";
    zoneSelector += "<div class='zoneSelectListHeader'>Select the Zone to add:";
    zoneSelector += "<input type='button' id='zones-select-add-all-" + element.id + "' class='blue forceAlignRight' value = 'Add All Zones'";
    zoneSelector += "onClick='addZoneToZonesSelection(" + '"' + element.id + '", "ALL"' + ");'>";
    zoneSelector += "</div>";
    zoneSelector += "<div class='zoneSelectList' id = 'zones-select-selector-list-" + element.id + "'>";
    let i, name;
    for (i = 0; i < zonesList.length; i++) {
        if (zonesList[i].name === "") {
            name = zonesList[i].id;
        } else {
            name = zonesList[i].name;
        }
        zoneSelector += "<button class='menu-button' onClick='addZoneToZonesSelection(";
        zoneSelector += '"' + element.id + '"' + ", " + '"' + zonesList[i].id + '"' + ");'>" + name + "</button><br>";
    }   
    zoneSelector += "</div>";
    zoneSelector += "<div class='zoneSelectListFooter'>";
    zoneSelector += "<input type='button' value = 'Cancel' id='zones-select-cancel-" + element.id + "' onClick='document.getElementById(";
    zoneSelector += '"' + "zones-select-selector-list-panel-" + element.id + '"' + ").style.display = " + '"' + "none" + '"' + ";'>";
    zoneSelector += "</div></div></div>";
    zoneSelector += "<div class='selected-zone-container' id='selected-zones-" + element.id + "'></div>";
    zoneSelectorContainer.innerHTML = zoneSelector;
    displaySelectedZones(element.id, isEditable);
}

function getZoneName(zoneID) {
    let name = "{ERROR: NOT FOUND}";
    let j;
    for (j = 0; j < zonesList.length; j++) {
        if (zoneID === zonesList[j].id) {
            if (zonesList[j].name !== "") {
                name = zonesList[j].name;
            } else {
                name = zonesList[j].id;
            }
            break;
        }
    }
    return name;
}

/**
 * ZONES COEFFICIENT
 */

function createZonesCoefficient(property, elProperty) {
    let propertyData = property.data;
    let elementID = property.elementID;
    elProperty.className = "zonesCoefficient";
    let elInput = document.createElement('input');
    elInput.setAttribute("id", elementID);
    elInput.setAttribute("type", "hidden");
    elInput.setAttribute("multipleZonesSelectionID", propertyData.multipleZonesSelectionID);
    elInput.className = "hiddenZonesCoefficient";

    let elZonesCoefficientSelector = document.createElement('div');
    elZonesCoefficientSelector.setAttribute("id", "zonesCoefficient-selector-" + elementID);

    let elMultiDiff = document.createElement('span');
    elMultiDiff.className = "multi-diff";

    elProperty.appendChild(elInput);
    elProperty.appendChild(elZonesCoefficientSelector);
    elProperty.appendChild(elMultiDiff);

    return elInput;
}

function setZonesCoefficientUi(propertyId, isEditable) {
    let i, listedCoefficientsInner, hiddenData, isMultiple, multipleZonesSelectionID, zoneListData;
    hiddenData = document.getElementById(propertyId).value;
    multipleZonesSelectionID = document.getElementById(propertyId).getAttribute('multipleZonesSelectionID');
    zoneListData = JSON.parse(document.getElementById("property-"  + multipleZonesSelectionID.replace(".", "-")).value);
    if (JSON.stringify(hiddenData) === '"undefined"') {
        isMultiple = true;
        hiddenData = "[]";
    } else {
        isMultiple = false;
    }
    listedCoefficientsInner = "<div class='zonesCoefficientContainer'>";
    let coefficients = JSON.parse(hiddenData);
    if (coefficients.length !== zoneListData.length) {
        listedCoefficientsInner += "<div class='zonesCoefficientWarning'>";
        listedCoefficientsInner += "WARNING: Zones and coefficients are desynchronized.<br>";
        listedCoefficientsInner += "<input type='button' value = 'Fix this' onClick='fixUnsynchZonesCoefficients(" + '"' + propertyId + '", ' + zoneListData.length + ");'>";
        listedCoefficientsInner += "</div>";
    } else {
        if (coefficients.length === 0) {
            if (isMultiple) {
                listedCoefficientsInner += "Not available.";
            }
            listedCoefficientsInner += "<br>";
        } else {
            let zoneName, lineColor;
            for (i = 0; i < coefficients.length; i++) {
                zoneName = getZoneName(zoneListData[i]);
                lineColor = "";
                if (i % 2 !== 0) {
                    lineColor = " zonesCoefficientDarkLine";
                }
                if (isEditable) {
                    listedCoefficientsInner += "<div class='zonesCoefficientLine" + lineColor + "'>";
                    listedCoefficientsInner += "<span class='zonesCoefficientCoef'>";
                    listedCoefficientsInner += "<input class='zonesCoefficient' type='number' step='0.005' max = '1.0' value='" + +coefficients[i].toFixed(4) + "' onChange='setZonesCoefficientValue(" + '"' + propertyId + '", ' + i + ", this.value);'>";
                    listedCoefficientsInner += "</span>";
                    listedCoefficientsInner += "<span class='zonesCoefficientZone'>&nbsp;" + zoneName + "</span>";
                    listedCoefficientsInner += "</div>";
                } else {
                    listedCoefficientsInner += "<div class='zonesCoefficientLine" + lineColor + "'>";
                    listedCoefficientsInner += "<span class='zonesCoefficientCoef'><div class='zonesCoefficientReadOnly'>" + +coefficients[i].toFixed(4) + "</div></span>";
                    listedCoefficientsInner += "<span class='zonesCoefficientZone'>&nbsp;&nbsp;" + zoneName + "</span>";
                    listedCoefficientsInner += "</div>";
                }
            }
        }
    }
    listedCoefficientsInner += "</div>";
    document.getElementById("zonesCoefficient-selector-" + propertyId).innerHTML = listedCoefficientsInner;
}

function fixUnsynchZonesCoefficients(propertyId, expectedLength) {
    let hiddenField = document.getElementById(propertyId);
    if (JSON.stringify(hiddenField.value) === '"undefined"') {
        hiddenField.value = "[]";
    }
    let originalCoefficients = JSON.parse(hiddenField.value);
    let coefficients = [];
    let i;
    for (i = 0; i < expectedLength; i++) {
        if ( i < originalCoefficients.length) {
            coefficients[i] = originalCoefficients[i];
        } else {
            coefficients[i] = AUDIO_ATTENUATION_COEFFICIENT_DEFAULT_VALUE;
        }
    }
    hiddenField.value = JSON.stringify(coefficients);
    
    let propertyName = propertyId.replace("property-", "");
    propertyName = propertyName.replace("-", ".");
    updateProperty(propertyName, coefficients, false);
}

function setZonesCoefficientValue(propertyId, index, coef) {
    function cleanFloat(val) {
        let num = parseFloat(val);
        if (Number.isNaN(num)) {
            return 0;
        }
        return num;
    }
    
    let newCoefValue = cleanFloat(coef);
    let hiddenField = document.getElementById(propertyId);
    if (JSON.stringify(hiddenField.value) === '"undefined"') {
        hiddenField.value = "[]";
    }
    let coefficients = JSON.parse(hiddenField.value);
    coefficients[index] = newCoefValue;
    hiddenField.value = JSON.stringify(coefficients);
    
    let propertyName = propertyId.replace("property-", "");
    propertyName = propertyName.replace("-", ".");
    updateProperty(propertyName, coefficients, false);

}

function removeElementFromZonesCoefficient(propertyId, index) {
    let hiddenField = document.getElementById(propertyId);
    if (JSON.stringify(hiddenField.value) === '"undefined"') {
        hiddenField.value = "[]";
    }
    let coefficients = JSON.parse(hiddenField.value);
    coefficients.splice(index, 1);
    hiddenField.value = JSON.stringify(coefficients);
    
    let propertyName = propertyId.replace("property-", "");
    propertyName = propertyName.replace("-", ".");
    updateProperty(propertyName, coefficients, false);
}

function addElementToZonesCoefficient(propertyId) {
    let hiddenField = document.getElementById(propertyId);
    if (JSON.stringify(hiddenField.value) === '"undefined"') {
        hiddenField.value = "[]";
    }
    let coefficients = JSON.parse(hiddenField.value);
    coefficients.push(AUDIO_ATTENUATION_COEFFICIENT_DEFAULT_VALUE);
    hiddenField.value = JSON.stringify(coefficients);
    
    let propertyName = propertyId.replace("property-", "");
    propertyName = propertyName.replace("-", ".");
    updateProperty(propertyName, coefficients, false);
}


/**
 * ARRAY-OF-STRINGS FUNCTIONS
 */

function createArrayOfStrings(property, elProperty) {
    let propertyData = property.data;
    let elementID = property.elementID;
    elProperty.className = "arrayOfStrings";
    let elInput = document.createElement('input');
    elInput.setAttribute("id", elementID);
    elInput.setAttribute("type", "hidden");
    elInput.setAttribute("useStringColor", propertyData.useStringColor);
    elInput.className = "hiddenArrayOfStrings";

    let elArrayOfStringsSelector = document.createElement('div');
    elArrayOfStringsSelector.setAttribute("id", "arrayOfStrings-selector-" + elementID);

    let elMultiDiff = document.createElement('span');
    elMultiDiff.className = "multi-diff";

    elProperty.appendChild(elInput);
    elProperty.appendChild(elArrayOfStringsSelector);
    elProperty.appendChild(elMultiDiff);

    return elInput;
}

function setArrayOfStringsUi(propertyId, isEditable) {
    let i, listedStringsInner, hiddenData, isMultiple, useStringColor, tagStyle;
    hiddenData = document.getElementById(propertyId).value;
    useStringColor = document.getElementById(propertyId).getAttribute('useStringColor').toLowerCase() === "true";
    if (JSON.stringify(hiddenData) === '"undefined"') {
        isMultiple = true;
        hiddenData = "[]";
    } else {
        isMultiple = false;
    }
    listedStringsInner = "<div class='arrayOfStringsContainer'>";
    let selectedStrings = JSON.parse(hiddenData);
    if (selectedStrings.length === 0) {
        if (isMultiple) {
            listedStringsInner += "<br>"; //or anything saying we dont suport multiple selection, but we might by the list with bulk actions.
        }
    } else {
        selectedStrings.sort();
        let counter = 0;
        for (i = 0; i < selectedStrings.length; i++) {
            tagStyle = "";
            if (useStringColor) {
                tagStyle = " style='color:#bbbbbb; background-color:" + getColorOfString(selectedStrings[i]) + ";'";
            }
            if (isEditable) {
                listedStringsInner += "<div class='arrayOfStringsTags'" + tagStyle + ">" + selectedStrings[i] + "&nbsp;&nbsp;"; 
                listedStringsInner += "<span class='arrayOfStringsTagsRemove' onClick='removeElementFromArrayOfStrings(" + '"' + propertyId + '"' + ", " + '"' + selectedStrings[i] + '"' + ");' >&#10006;</span>"
                listedStringsInner += "</div>";
            } else {
                listedStringsInner += "<div class='arrayOfStringsTags'" + tagStyle + ">" + selectedStrings[i] + "</div>";
            }
            counter++;
            if (counter === MAX_TAGS_PER_ROWS) {
                listedStringsInner += "<br>";
                counter = 0;
            }
        }
    }
    if (isEditable && !isMultiple) {
        let addComponent = "<div class='arrayOfStringsAddComponentContainer'><input class='arrayOfStringsStringToAdd' type='text' id='arrayOfStringsStringToAdd-" + propertyId + "'>";
        addComponent += " <input type='button' class='glyph' value = 'K' id='arrayOfStringsAddButton-" + propertyId + "' onClick='addElementToArrayOfStrings(" + '"' + propertyId + '", "';
        addComponent += "arrayOfStringsStringToAdd-" + propertyId + '"' + ");'>";
        addComponent += "</div>";
        listedStringsInner += addComponent;
    }
    listedStringsInner += "</div>";
    document.getElementById("arrayOfStrings-selector-" + propertyId).innerHTML = listedStringsInner;
}

function removeElementFromArrayOfStrings(propertyId, stringText) {
    let hiddenField = document.getElementById(propertyId);
    if (JSON.stringify(hiddenField.value) === '"undefined"') {
        hiddenField.value = "[]";
    }
    let selectedStrings = JSON.parse(hiddenField.value);
    let index = selectedStrings.indexOf(stringText);
    if (index > -1) {
      selectedStrings.splice(index, 1);
    }  
    hiddenField.value = JSON.stringify(selectedStrings);
    setArrayOfStringsUi(propertyId, true);
    let propertyName = propertyId.replace("property-", "");
    propertyName = propertyName.replace("-", ".");
    updateProperty(propertyName, selectedStrings, false);
}

function addElementToArrayOfStrings(propertyId, elementToGetText) {
    let stringText = document.getElementById(elementToGetText).value;
    if (stringText === "") {
        return;
    } else {
        let hiddenField = document.getElementById(propertyId);
        if (JSON.stringify(hiddenField.value) === '"undefined"') {
            hiddenField.value = "[]";
        }
        let selectedStrings = JSON.parse(hiddenField.value);
        if (!selectedStrings.includes(stringText)) {
            selectedStrings.push(stringText);
        }
        hiddenField.value = JSON.stringify(selectedStrings);
        setArrayOfStringsUi(propertyId, true);
        let propertyName = propertyId.replace("property-", "");
        propertyName = propertyName.replace("-", ".");
        updateProperty(propertyName, selectedStrings, false);
    }
}

function getColorOfString(input) {
    let sum = 0;
    for (let i = 0; i < input.length; i++) {
        sum += input.charCodeAt(i);
    }
    let hue = sum % 360;
    return hslToHex(hue, 100, 20);
}

function hslToHex(h, s, l) {
    s /= 100;
    l /= 100;
    const c = (1 - Math.abs(2 * l - 1)) * s;
    const x = c * (1 - Math.abs((h / 60) % 2 - 1));
    const m = l - c / 2;
    let r = 0, g = 0, b = 0;
    if (0 <= h && h < 60) {
        [r, g, b] = [c, x, 0];
    } else if (60 <= h && h < 120) {
        [r, g, b] = [x, c, 0];
    } else if (120 <= h && h < 180) {
        [r, g, b] = [0, c, x];
    } else if (180 <= h && h < 240) {
        [r, g, b] = [0, x, c];
    } else if (240 <= h && h < 300) {
        [r, g, b] = [x, 0, c];
    } else if (300 <= h && h < 360) {
        [r, g, b] = [c, 0, x];
    }
    const toHex = (n) => {
        const hex = Math.round((n + m) * 255).toString(16);
        return hex.padStart(2, '0');
    };
    return `#${toHex(r)}${toHex(g)}${toHex(b)}`;
}

/**
 * CHILD ENTITIES FUNCTIONS
 */
 
function createChildList(property, elProperty) {
    let elementID = property.elementID;
    elProperty.className = "childEntityList";

    let elInput = document.createElement('div');
    elInput.setAttribute("id", "childList-" + elementID);

    elProperty.appendChild(elInput);
    return elInput;
}

function setChildListData(element, children, parentID, entityHostType = "") {
    
    let childListContainer = document.getElementById(element.id);
    let renderer = "";
    let i;
    renderer += "<div id='childEntityActionsContainer'>";
    if (parentID !== "") {
        renderer += "<div id='viewParentContainer'><span class='viewParent' onClick='navigateToSpecificEntity(" + '"'+ parentID + '"'+ ")'><font class='viewParentIcon'>1</font>View Parent</span></div>";
    } else {
        renderer += "<div id='viewParentContainer'></div>";
    }
    if ((entityHostType === "domain" || entityHostType === "avatar") && children !== undefined) {
        renderer += "<div id='addChildEntityContainer'><span class='viewParent' onClick='openCreateChildEntityAssistant(" + '"' + entityHostType + '"' + ")'>+ Add Child Entity&nbsp;&nbsp;</span></div>";
    } else {
        renderer += "<div id='addChildEntityContainer'></div>";
    }
    renderer += "</div>";
    
    renderer += "<table>";
    renderer += "<tr><th class='childrenTableHeader' width='30%'>TYPE</th><th class='childrenTableHeader' width='65%'>NAME</th><th class='childrenTableHeader' width='5%'>VIEW</th></tr>";
    if (children === undefined) {
        renderer += "<tr><td colspan = '3' style='text-align: center;'><i>Not applicable</i></td></tr>";
    } else {
        if (children.length > 0) {
            for (i = 0; i < children.length; i++ ) {
                let entityHostTypeClass = "";
                if (children[i].entityHostType !== "domain") {
                    entityHostTypeClass = " class='" + children[i].entityHostType + "Entity'";
                }
                let navigatorBtn = "<span class='viewChildProperties' onClick='navigateToSpecificEntity(" + '"'+ children[i].id + '"'+ ")'>&#129094;</span>";
                renderer += "<tr" + entityHostTypeClass + "><td>" + children[i].type + "</td><td>" + children[i].name + "</td><td>" + navigatorBtn + "</td></tr>";
            }
        } else {
            renderer += "<tr><td colspan = '3' style='text-align: center;'><i>No children</i></td></tr>";
        }
    }
    renderer += "</table>";
    childListContainer.innerHTML = renderer;
}

function navigateToSpecificEntityFromParentID() {
    let parentID = getPropertyInputElement("parentID").value;
    if (parentID !== "" && parentID !== UUID_NONE) {
        navigateToSpecificEntity(parentID);
    }
}

function setParentIdNavigationAvailable(selectionLength) {
    $('#property-parentID-button-navigateToParentEntity').attr('disabled', selectionLength !== 1);
}

function navigateToSpecificEntity(id) {
    EventBridge.emitWebEvent(JSON.stringify({
        type: "specificEntityNavigation",
        id: id
    }));
}

//CREATE CHILD ENTITY FUNCTIONS:

function openCreateChildEntityAssistant(entityHostType) {
    $('#uiCreateChildEntityAssistant').show();
    $('#properties-list').hide();
    generateCreateChildEntityAssistant(entityHostType);
}

function generateCreateChildEntityAssistant(entityHostType) {
    let entityType = [
        {"type": "Model", "name": "Model"},
        {"type": "Shape", "name": "Shape"},
        {"type": "Text", "name": "Text"},
        {"type": "Image", "name": "Image"},
        {"type": "Web", "name": "Web"},
        {"type": "ParticleEffect", "name": "Particle"},
        {"type": "ProceduralParticleEffect", "name": "Proc. Part."},
        {"type": "Light", "name": "Light"},
        {"type": "Zone", "name": "Zone"},
        {"type": "Material", "name": "Material"},
        {"type": "Sound", "name": "Sound"},
        {"type": "PolyVox", "name": "Voxel"},
    ];
    const TILES_PER_ROW = 4;
    let renderer = "<div id='typeSelectorCreateChildEntityAssistant' style = 'display: block;'>";
    let i;
    let rowCount = 0;
    for (i = 0; i < entityType.length; i++) {
        if (rowCount === 0) {
            renderer += "<div>";
        }
        rowCount++;
        renderer += "<span class='tileCreateChildEntityAssistant-" + entityHostType + "' onclick='selectTypeInChildEntityAssistant(" + '"' +  entityType[i].type + '"' +  ", " + '"' +  entityHostType + '"' + ")'>";
        renderer += "<font class = 'iconCreateChildEntityAssistant'>" + ENTITY_TYPE_ICON[entityType[i].type] + "</font><br>" + entityType[i].name.toUpperCase() + "</span>";
        if (rowCount === TILES_PER_ROW) {
            renderer += "</div>";
            rowCount = 0;
        }
    }
    if (rowCount !== 0) {
        renderer += "</div>";
    }
    renderer += "</div>";
    
    renderer += "<div id='paramaterCreateChildEntityAssistant' style='display: none;'>";
    renderer += "<div id='nameCreateChildEntityAssistant'></div><br>";
    renderer += "<font class='addChildEntity-" + entityHostType + "'>URL: </font><input id='urlCreateChildEntityAssistant' type='text'><br><br>";
    renderer += "<div style='text-align: right;'>";
    renderer += "<input type='button' class='black' id='cancelBtnCreateChildEntityAssistant' value='Cancel'>&nbsp;&nbsp;";
    renderer += "<input type='button' class='white' id='createBtnCreateChildEntityAssistant' value='Create'>";
    renderer += "<div>";
    renderer += "</div>";
    
    document.getElementById("uiCreateChildEntityAssistant-form").innerHTML = renderer;
}

function selectTypeInChildEntityAssistant(type, entityHostType) {
    if (type === "Model" || type === "Sound") { //Only if the url is cruxial. We want the less entry as possible for the user.
        document.getElementById("nameCreateChildEntityAssistant").innerHTML = "<br><br><font class='addChildEntity-" + entityHostType + "'>Create a child '" + type + "' entity<br>";
        document.getElementById("typeSelectorCreateChildEntityAssistant").style.display = "none";
        document.getElementById("paramaterCreateChildEntityAssistant").style.display = "block";
        document.getElementById("createBtnCreateChildEntityAssistant").setAttribute("onclick","createChildEntity('" + type + "', '" + entityHostType + "')");
        document.getElementById("cancelBtnCreateChildEntityAssistant").setAttribute("onclick","closeCreateChildEntityAssistant()");
    } else {
        createChildEntity(type, entityHostType);
    }
}

function createChildEntity(type, entityHostType) {
    let url = document.getElementById("urlCreateChildEntityAssistant").value;
    let parentID = getPropertyInputElement("id").value;
    let properties;
    switch(type) {
        case "Model":
            properties = {
                "type": type,
                "modelURL": url,
                "parentID": parentID,
                "shapeType": "static-mesh",
                "dynamic": false,
                "grab": {"grabbable": false},
                "useOriginalPivot": true
            };
            break;
        case "Shape":
            properties = {
                "type": type,
                "parentID": parentID,
                "shape": "Cube"
            };
            break;
        case "Text":
            properties = {
                "type": type,
                "parentID": parentID
            };
            break;
        case "Image":
            properties = {
                "type": type,
                "parentID": parentID
            };
            break;
        case "Web":
            properties = {
                "type": type,
                "parentID": parentID
            };
            break;
        case "ParticleEffect":
            properties = {
                "type": type,
                "parentID": parentID
            };
            break;
        case "ProceduralParticleEffect":
            properties = {
                "type": type,
                "parentID": parentID
            };
            break;
        case "Light":
            properties = {
                "type": type,
                "parentID": parentID
            };
            break;
        case "Zone":
            properties = {
                "type": type,
                "parentID": parentID
            };
            break;
        case "Material":
            properties = {
                "type": type,
                "materialURL": "materialData",
                "materialData": JSON.stringify({ "materials": {} }),
                "parentID": parentID,
                "priority": 1
            };
            break;
        case "Sound":
            properties = {
                "type": type,
                "soundURL": url,
                "parentID": parentID
            };
            break;
        case "PolyVox":
            properties = {
                "type": type,
                "parentID": parentID
            };
            break;
    }

    EventBridge.emitWebEvent(JSON.stringify({
        "type": "createChildEntity",
        "properties": properties,
        "entityHostType": entityHostType
    }));
   
    closeCreateChildEntityAssistant();
}

function closeCreateChildEntityAssistant() {
    $('#uiCreateChildEntityAssistant').hide();
    $('#properties-list').show();
}

/**
 * MATERIAL TARGET FUNCTIONS
 */

function requestMaterialTarget() {
    EventBridge.emitWebEvent(JSON.stringify({
        type: 'materialTargetRequest',
        entityID: getFirstSelectedID(),
    }));
}

function setMaterialTargetData(materialTargetData) {
    let elDivOptions = getPropertyInputElement("parentMaterialName");
    resetDynamicMultiselectProperty(elDivOptions);

    if (materialTargetData === undefined) {
        return;
    }

    elDivOptions.firstChild.style.display = "none"; // hide "No Options" text
    elDivOptions.parentNode.lastChild.style.display = null; // show Select/Clear all buttons

    let numMeshes = materialTargetData.numMeshes;
    for (let i = 0; i < numMeshes; ++i) {
        addMaterialTarget(elDivOptions, i, false);
    }

    let materialNames = materialTargetData.materialNames;
    let materialNamesAdded = [];
    for (let i = 0; i < materialNames.length; ++i) {
        let materialName = materialNames[i];
        if (materialNamesAdded.indexOf(materialName) === -1) {
            addMaterialTarget(elDivOptions, materialName, true);
            materialNamesAdded.push(materialName);
        }
    }

    materialTargetPropertyUpdate(elDivOptions.propertyValue);
}

function addMaterialTarget(elDivOptions, targetID, isMaterialName) {
    let elementID = elDivOptions.getAttribute("id");
    elementID += isMaterialName ? "-material-" : "-mesh-";
    elementID += targetID;

    let elDiv = document.createElement('div');
    elDiv.className = "materialTargetDiv";
    elDiv.onclick = onToggleMaterialTarget;
    elDivOptions.appendChild(elDiv);

    let elInput = document.createElement('input');
    elInput.className = "materialTargetInput";
    elInput.setAttribute("type", "checkbox");
    elInput.setAttribute("id", elementID);
    elInput.setAttribute("targetID", targetID);
    elInput.setAttribute("isMaterialName", isMaterialName);
    elDiv.appendChild(elInput);

    let elLabel = document.createElement('label');
    elLabel.setAttribute("for", elementID);
    elLabel.innerText = isMaterialName ? "Material " + targetID : "Mesh Index " + targetID;
    elDiv.appendChild(elLabel);

    return elDiv;
}

function onToggleMaterialTarget(event) {
    let elTarget = event.target;
    if (elTarget instanceof HTMLInputElement) {
        sendMaterialTargetProperty();
    }
    event.stopPropagation();
}

function setAllMaterialTargetInputs(checked) {
    let elDivOptions = getPropertyInputElement("parentMaterialName");   
    let elInputs = elDivOptions.getElementsByClassName("materialTargetInput");
    for (let i = 0; i < elInputs.length; ++i) {
        elInputs[i].checked = checked;
    }
}

function selectAllMaterialTarget() {
    setAllMaterialTargetInputs(true);
    sendMaterialTargetProperty();
}

function clearAllMaterialTarget() {
    setAllMaterialTargetInputs(false);
    sendMaterialTargetProperty();
}

function sendMaterialTargetProperty() {
    let elDivOptions = getPropertyInputElement("parentMaterialName");   
    let elInputs = elDivOptions.getElementsByClassName("materialTargetInput");

    let materialTargetList = [];
    for (let i = 0; i < elInputs.length; ++i) {
        let elInput = elInputs[i];
        if (elInput.checked) {
            let targetID = elInput.getAttribute("targetID");
            if (elInput.getAttribute("isMaterialName") === "true") {
                materialTargetList.push(MATERIAL_PREFIX_STRING + targetID);
            } else {
                materialTargetList.push(targetID);
            }
        }
    }

    let propertyValue = materialTargetList.join(",");
    if (propertyValue.length > 1) {
        propertyValue = "[" + propertyValue + "]";
    }

    updateProperty("parentMaterialName", propertyValue, false);
}

function materialTargetPropertyUpdate(propertyValue) {
    let elDivOptions = getPropertyInputElement("parentMaterialName");
    let elInputs = elDivOptions.getElementsByClassName("materialTargetInput");

    if (propertyValue.startsWith('[')) {
        propertyValue = propertyValue.substring(1, propertyValue.length);
    }
    if (propertyValue.endsWith(']')) {
        propertyValue = propertyValue.substring(0, propertyValue.length - 1);
    }

    let materialTargets = propertyValue.split(",");
    for (let i = 0; i < elInputs.length; ++i) {
        let elInput = elInputs[i];
        let targetID = elInput.getAttribute("targetID");
        let materialTargetName = targetID;
        if (elInput.getAttribute("isMaterialName") === "true") {
            materialTargetName = MATERIAL_PREFIX_STRING + targetID;
        }
        elInput.checked = materialTargets.indexOf(materialTargetName) >= 0;
    }

    elDivOptions.propertyValue = propertyValue;
}

function roundAndFixNumber(number, propertyData) {
    let result = number;
    if (propertyData.round !== undefined) {
        result = Math.round(result * propertyData.round) / propertyData.round;
    }
    if (propertyData.decimals !== undefined) {
        return result.toFixed(propertyData.decimals)
    }
    return result;
}

function applyInputNumberPropertyModifiers(number, propertyData) {
    const multiplier = propertyData.multiplier !== undefined ? propertyData.multiplier : 1;
    return roundAndFixNumber(number / multiplier, propertyData);
}

function applyOutputNumberPropertyModifiers(number, propertyData) {
    const multiplier = propertyData.multiplier !== undefined ? propertyData.multiplier : 1;
    return roundAndFixNumber(number * multiplier, propertyData);
}

const areSetsEqual = (a, b) => a.size === b.size && [...a].every(value => b.has(value));


function handleEntitySelectionUpdate(selections, isPropertiesToolUpdate) {
    const previouslySelectedEntityIDs = selectedEntityIDs;
    currentSelections = selections;
    selectedEntityIDs = new Set(selections.map(selection => selection.id));
    const multipleSelections = currentSelections.length > 1;
    const hasSelectedEntityChanged = !areSetsEqual(selectedEntityIDs, previouslySelectedEntityIDs);
    
    if (selections.length === 1) {
        if (maSelectedId !== selections[0].id) {
            closeMaterialAssistant();
        }
        maSelectedId = selections[0].id;
        if (selections[0].properties.type === "Zone") {
            skyboxColorForCopy = selections[0].properties.skybox.color;
        } else {
            skyboxColorForCopy = undefined;
        }
    } else {
        closeMaterialAssistant();
        maSelectedId = "";
        skyboxColorForCopy = undefined;
    }

    requestZoneList();
    
    if (selections.length === 0) {
        deleteJSONEditor();
        deleteJSONMaterialEditor();
        deleteJSONParticleUpdateEditor();
        deleteJSONParticleRenderEditor();

        resetProperties();
        showGroupsForType("None");

        let elIcon = properties.type.elSpan;
        elIcon.innerText = NO_SELECTION;
        elIcon.style.display = 'inline-block';

        getPropertyInputElement("userData").value = "";
        showUserDataTextArea();
        showSaveUserDataButton();
        showNewJSONEditorButton();

        getPropertyInputElement("materialData").value = "";
        showMaterialDataTextArea();
        showSaveMaterialDataButton();
        showNewJSONMaterialEditorButton();

        getPropertyInputElement("particleUpdateData").value = "";
        showParticleUpdateDataTextArea();
        showSaveParticleUpdateDataButton();
        showNewJSONParticleUpdateEditorButton();

        getPropertyInputElement("particleRenderData").value = "";
        showParticleRenderDataTextArea();
        showSaveParticleRenderDataButton();
        showNewJSONParticleRenderEditorButton();

        setCopyPastePositionAndRotationAvailability (selections.length, true);

        disableProperties();
        
        setParentIdNavigationAvailable(selections.length);
    } else {
        let entityHostType = selections[0].properties.entityHostType;
        
        if (!isPropertiesToolUpdate && !hasSelectedEntityChanged && document.hasFocus()) {
            // in case the selection has not changed and we still have focus on the properties page,
            // we will ignore the event.
            return;
        }

        if (hasSelectedEntityChanged) {
            closeCreateChildEntityAssistant();
            if (!multipleSelections) {
                resetServerScriptStatus();
            }
        }

        const doSelectElement = !hasSelectedEntityChanged;

        // Get unique entity types, and convert the types Sphere and Box to Shape
        const shapeTypes = ["Sphere", "Box"];
        const entityTypes = [...new Set(currentSelections.map(a =>
            shapeTypes.includes(a.properties.type) ? "Shape" : a.properties.type))];

        const shownGroups = getGroupsForTypes(entityTypes);
        showGroupsForTypes(entityTypes);
        showOnTheSamePage(entityTypes);

        const lockedMultiValue = getMultiplePropertyValue('locked');

        if (lockedMultiValue.isMultiDiffValue || lockedMultiValue.value) {
            disableProperties();
            getPropertyInputElement('locked').removeAttribute('disabled');
            setCopyPastePositionAndRotationAvailability (selections.length, true);
            setParentIdNavigationAvailable(selections.length);
        } else {
            enableProperties();
            disableSaveUserDataButton();
            disableSaveMaterialDataButton();
            disableSaveParticleUpdateDataButton();
            disableSaveParticleRenderDataButton();
            setCopyPastePositionAndRotationAvailability (selections.length, false);
            setParentIdNavigationAvailable(selections.length);
        }

        Object.entries(properties).forEach(function([propertyID, property]) {
            const propertyData = property.data;
            const propertyName = property.name;
            let propertyMultiValue = getMultiplePropertyValue(propertyName);
            let isMultiDiffValue = propertyMultiValue.isMultiDiffValue;
            let propertyValue = propertyMultiValue.value;

            if (propertyData.selectionVisibility !== undefined) {
                let visibility = propertyData.selectionVisibility;
                let propertyVisible = true;
                if (!multipleSelections) {
                    propertyVisible = isFlagSet(visibility, PROPERTY_SELECTION_VISIBILITY.SINGLE_SELECTION);
                } else if (isMultiDiffValue) {
                    propertyVisible = isFlagSet(visibility, PROPERTY_SELECTION_VISIBILITY.MULTI_DIFF_SELECTIONS);
                } else {
                    propertyVisible = isFlagSet(visibility, PROPERTY_SELECTION_VISIBILITY.MULTIPLE_SELECTIONS);
                }
                setPropertyVisibility(property, propertyVisible);
            }

            const isSubProperty = propertyData.subPropertyOf !== undefined;
            if (propertyValue === undefined && !isMultiDiffValue && !isSubProperty) {
                if (propertyData.type !== "childList") {
                    return;
                }
            }

            if (!shownGroups.includes(property.group_id)) {
                const WANT_DEBUG_SHOW_HIDDEN_FROM_GROUPS = false;
                if (WANT_DEBUG_SHOW_HIDDEN_FROM_GROUPS) {
                    console.log("Skipping property " + property.data.label + " [" + property.name +
                        "] from hidden group " + property.group_id);
                }
                return;
            }

            if (propertyName === "type") {
                propertyValue = entityTypes.length > 1 ?  "Multiple" : propertyMultiValue.values[0];
            }

            switch (propertyData.type) {
                case 'string': {
                    if (isMultiDiffValue) {
                        if (propertyData.readOnly && propertyData.multiDisplayMode
                            && propertyData.multiDisplayMode === PROPERTY_MULTI_DISPLAY_MODE.COMMA_SEPARATED_VALUES) {
                            property.elInput.value = propertyMultiValue.values.join(", ");
                        } else {
                            property.elInput.classList.add('multi-diff');
                            property.elInput.value = "";
                        }
                        if (propertyName === "id") {
                            property.elInput.style.color = ENTITY_HOST_TYPE_COLOR_DOMAIN;
                        }
                    } else {
                        property.elInput.classList.remove('multi-diff');
                        property.elInput.value = propertyValue;
                        if (propertyName === "name" || propertyName === "id") {
                            if (selections.length === 1) {
                                switch (entityHostType) {
                                    case "domain":
                                        property.elInput.style.color = ENTITY_HOST_TYPE_COLOR_DOMAIN;
                                        break;
                                    case "avatar":
                                        property.elInput.style.color = ENTITY_HOST_TYPE_COLOR_AVATAR;
                                        break;
                                    case "local":
                                        property.elInput.style.color = ENTITY_HOST_TYPE_COLOR_LOCAL;
                                        break;
                                    default:
                                        property.elInput.style.color = ENTITY_HOST_TYPE_COLOR_DOMAIN;
                                }
                            } else {
                                property.elInput.style.color = ENTITY_HOST_TYPE_COLOR_DOMAIN;
                            }
                        }
                    }
                    break;
                }
                case 'bool': {
                    const inverse = propertyData.inverse !== undefined ? propertyData.inverse : false;
                    if (isSubProperty) {
                        let subPropertyMultiValue = getMultiplePropertyValue(propertyData.subPropertyOf);
                        let propertyValue = subPropertyMultiValue.value;
                        isMultiDiffValue = subPropertyMultiValue.isMultiDiffValue;
                        if (isMultiDiffValue) {
                            let detailedSubProperty = getDetailedSubPropertyMPVDiff(subPropertyMultiValue, propertyName);
                            property.elInput.checked = detailedSubProperty.isChecked;
                            property.elInput.classList.toggle('multi-diff', detailedSubProperty.isMultiDiff);
                        } else {
                            let subProperties = propertyValue.split(",");
                            let subPropertyValue = subProperties.indexOf(propertyName) > -1;
                            property.elInput.checked = inverse ? !subPropertyValue : subPropertyValue;
                            property.elInput.classList.remove('multi-diff');
                        }

                    } else {
                        if (isMultiDiffValue) {
                            property.elInput.checked = false;
                        } else {
                            property.elInput.checked = inverse ? !propertyValue : propertyValue;
                        }
                        property.elInput.classList.toggle('multi-diff', isMultiDiffValue);
                    }

                    break;
                }
                case 'number': {
                    property.elInput.value = isMultiDiffValue ? "" : propertyValue;
                    property.elInput.classList.toggle('multi-diff', isMultiDiffValue);
                    break;
                }
                case 'number-draggable': {
                    let detailedNumberDiff = getDetailedNumberMPVDiff(propertyMultiValue, propertyData);
                    property.elNumber.setValue(detailedNumberDiff.averagePerPropertyComponent[0], detailedNumberDiff.propertyComponentDiff[0]);
                    break;
                }
                case 'rect': {
                    let detailedNumberDiff = getDetailedNumberMPVDiff(propertyMultiValue, propertyData);
                    property.elNumberX.setValue(detailedNumberDiff.averagePerPropertyComponent.x, detailedNumberDiff.propertyComponentDiff.x);
                    property.elNumberY.setValue(detailedNumberDiff.averagePerPropertyComponent.y, detailedNumberDiff.propertyComponentDiff.y);
                    property.elNumberWidth.setValue(detailedNumberDiff.averagePerPropertyComponent.width, detailedNumberDiff.propertyComponentDiff.width);
                    property.elNumberHeight.setValue(detailedNumberDiff.averagePerPropertyComponent.height, detailedNumberDiff.propertyComponentDiff.height);
                    break;
                }
                case 'vec3':
                case 'vec2': {
                    let detailedNumberDiff = getDetailedNumberMPVDiff(propertyMultiValue, propertyData);
                    property.elNumberX.setValue(detailedNumberDiff.averagePerPropertyComponent.x, detailedNumberDiff.propertyComponentDiff.x);
                    property.elNumberY.setValue(detailedNumberDiff.averagePerPropertyComponent.y, detailedNumberDiff.propertyComponentDiff.y);
                    if (property.elNumberZ !== undefined) {
                        property.elNumberZ.setValue(detailedNumberDiff.averagePerPropertyComponent.z, detailedNumberDiff.propertyComponentDiff.z);
                    }
                    break;
                }
                case 'color': {
                    let displayColor = propertyMultiValue.isMultiDiffValue ? propertyMultiValue.values[0] : propertyValue;
                    property.elColorPicker.style.backgroundColor = "rgb(" + displayColor.red + "," +
                        displayColor.green + "," +
                        displayColor.blue + ")";
                    property.elColorPicker.classList.toggle('multi-diff', propertyMultiValue.isMultiDiffValue);

                    if (hasSelectedEntityChanged && $(property.elColorPicker).attr('active') === 'true') {
                        // Set the color picker inactive before setting the color,
                        // otherwise an update will be sent directly after setting it here.
                        $(property.elColorPicker).attr('active', 'false');
                        colorPickers['#' + property.elementID].colpickSetColor({
                            "r": displayColor.red,
                            "g": displayColor.green,
                            "b": displayColor.blue
                        });
                        $(property.elColorPicker).attr('active', 'true');
                    }

                    property.elNumberR.setValue(displayColor.red);
                    property.elNumberG.setValue(displayColor.green);
                    property.elNumberB.setValue(displayColor.blue);
                    break;
                }
                case 'vec3rgb': {
                    let detailedNumberDiff = getDetailedNumberMPVDiff(propertyMultiValue, propertyData);
                    property.elNumberR.setValue(detailedNumberDiff.averagePerPropertyComponent.red, detailedNumberDiff.propertyComponentDiff.red);
                    property.elNumberG.setValue(detailedNumberDiff.averagePerPropertyComponent.green, detailedNumberDiff.propertyComponentDiff.green);
                    property.elNumberB.setValue(detailedNumberDiff.averagePerPropertyComponent.blue, detailedNumberDiff.propertyComponentDiff.blue);
                    break;
                }         
                case 'dropdown': {
                    property.elInput.classList.toggle('multi-diff', isMultiDiffValue);
                    property.elInput.value = isMultiDiffValue ? "" : propertyValue;
                    setDropdownText(property.elInput);
                    break;
                }
                case 'textarea':
                case 'code': {
                    property.elInput.value = propertyValue;
                    setTextareaScrolling(property.elInput);
                    break;
                }
                case 'multipleZonesSelection': {
                    property.elInput.value = JSON.stringify(propertyValue);
                    if (lockedMultiValue.isMultiDiffValue || lockedMultiValue.value) {
                        setZonesSelectionData(property.elInput, false);
                    } else {
                        setZonesSelectionData(property.elInput, true);
                    }
                    break;
                }
                case 'arrayOfStrings': {
                    property.elInput.value = JSON.stringify(propertyValue);
                    if (lockedMultiValue.isMultiDiffValue || lockedMultiValue.value) {
                        setArrayOfStringsUi(property.elInput.id, false);
                    } else {
                        setArrayOfStringsUi(property.elInput.id, true);
                    }
                    break;
                }
                case 'zonesCoefficient': {
                    property.elInput.value =  JSON.stringify(propertyValue);
                    if (lockedMultiValue.isMultiDiffValue || lockedMultiValue.value) {
                        setZonesCoefficientUi(property.elInput.id, false);
                    } else {
                        setZonesCoefficientUi(property.elInput.id, true);
                    }
                    break;
                }
                case 'childList': {
                    let parentID = selections[0].properties.parentID;
                    if (selections.length !== 1 || parentID === UUID_NONE) {
                        parentID = "";
                    }
                    setChildListData(property.elInput, propertyValue, parentID, entityHostType);
                    break;
                }
                case 'icon': {
                    property.elSpan.innerHTML = propertyData.icons[propertyValue];
                    property.elSpan.style.display = "inline-block";
                    break;
                }
                case 'texture': {
                    property.elInput.value = isMultiDiffValue ? "" : propertyValue;
                    property.elInput.classList.toggle('multi-diff', isMultiDiffValue);
                    if (isMultiDiffValue) {
                        property.elInput.setMultipleValues();
                    } else {
                        property.elInput.imageLoad(property.elInput.value);
                    }
                    break;
                }
                case 'dynamic-multiselect': {
                    if (!isMultiDiffValue && property.data.propertyUpdate) {
                        property.data.propertyUpdate(propertyValue);
                    }
                    break;
                }
            }

            let showPropertyRules = property.showPropertyRules;
            if (showPropertyRules !== undefined) {
                for (let propertyToShow in showPropertyRules) {
                    let showIfThisPropertyValue = showPropertyRules[propertyToShow];
                    let show = String(propertyValue) === String(showIfThisPropertyValue);
                    showPropertyElement(propertyToShow, show);
                }
            }
        });

        updateVisibleSpaceModeProperties();

        let userDataMultiValue = getMultiplePropertyValue("userData");
        let userDataTextArea = getPropertyInputElement("userData");
        let json = null;
        if (!userDataMultiValue.isMultiDiffValue) {
            try {
                json = JSON.parse(userDataMultiValue.value);
            } catch (e) {

            }
        }
        if (json !== null && !lockedMultiValue.isMultiDiffValue && !lockedMultiValue.value) {
            if (editor === null) {
                createJSONEditor();
            }
            userDataTextArea.classList.remove('multi-diff');
            setEditorJSON(json);
            showSaveUserDataButton();
            hideUserDataTextArea();
            hideNewJSONEditorButton();
            hideUserDataSaved();
        } else {
            // normal text
            deleteJSONEditor();
            userDataTextArea.classList.toggle('multi-diff', userDataMultiValue.isMultiDiffValue);
            userDataTextArea.value = userDataMultiValue.isMultiDiffValue ? "" : userDataMultiValue.value;

            showUserDataTextArea();
            showNewJSONEditorButton();
            hideSaveUserDataButton();
            hideUserDataSaved();
        }

        let materialDataMultiValue = getMultiplePropertyValue("materialData");
        let materialDataTextArea = getPropertyInputElement("materialData");
        let materialJSON = null;
        if (!materialDataMultiValue.isMultiDiffValue) {
            try {
                materialJSON = JSON.parse(materialDataMultiValue.value);
            } catch (e) {

            }
        }
        if (materialJSON !== null && !lockedMultiValue.isMultiDiffValue && !lockedMultiValue.value) {
            if (materialEditor === null) {
                createJSONMaterialEditor();
            }
            materialDataTextArea.classList.remove('multi-diff');
            setMaterialEditorJSON(materialJSON);
            showSaveMaterialDataButton();
            hideMaterialDataTextArea();
            hideNewJSONMaterialEditorButton();
            hideMaterialDataSaved();
        } else {
            // normal text
            deleteJSONMaterialEditor();
            materialDataTextArea.classList.toggle('multi-diff', materialDataMultiValue.isMultiDiffValue);
            materialDataTextArea.value = materialDataMultiValue.isMultiDiffValue ? "" :  materialDataMultiValue.value;
            showMaterialDataTextArea();
            showNewJSONMaterialEditorButton();
            hideSaveMaterialDataButton();
            hideMaterialDataSaved();
        }

        maGetMaterialDataAssistantAvailability(materialJSON);

        if (hasSelectedEntityChanged && selections.length === 1 && entityTypes[0] === "Material") {
            requestMaterialTarget();
        }

        let particleUpdateDataMultiValue = getMultiplePropertyValue("particleUpdateData");
        let particleUpdateDataTextArea = getPropertyInputElement("particleUpdateData");
        let particleUpdateJSON = null;
        if (!particleUpdateDataMultiValue.isMultiDiffValue) {
            try {
                particleUpdateJSON = JSON.parse(particleUpdateDataMultiValue.value);
            } catch (e) {

            }
        }
        if (particleUpdateJSON !== null && !lockedMultiValue.isMultiDiffValue && !lockedMultiValue.value) {
            if (particleUpdateEditor === null) {
                createJSONParticleUpdateEditor();
            }
            particleUpdateDataTextArea.classList.remove('multi-diff');
            setParticleUpdateEditorJSON(particleUpdateJSON);
            showSaveParticleUpdateDataButton();
            hideParticleUpdateDataTextArea();
            hideNewJSONParticleUpdateEditorButton();
            hideParticleUpdateDataSaved();
        } else {
            // normal text
            deleteJSONParticleUpdateEditor();
            particleUpdateDataTextArea.classList.toggle('multi-diff', particleUpdateDataMultiValue.isMultiDiffValue);
            particleUpdateDataTextArea.value = particleUpdateDataMultiValue.isMultiDiffValue ? "" : particleUpdateDataMultiValue.value;

            showParticleUpdateDataTextArea();
            showNewJSONParticleUpdateEditorButton();
            hideSaveParticleUpdateDataButton();
            hideParticleUpdateDataSaved();
        }

        let particleRenderDataMultiValue = getMultiplePropertyValue("particleRenderData");
        let particleRenderDataTextArea = getPropertyInputElement("particleRenderData");
        let particleRenderJSON = null;
        if (!particleRenderDataMultiValue.isMultiDiffValue) {
            try {
                particleRenderJSON = JSON.parse(particleRenderDataMultiValue.value);
            } catch (e) {

            }
        }
        if (particleRenderJSON !== null && !lockedMultiValue.isMultiDiffValue && !lockedMultiValue.value) {
            if (particleRenderEditor === null) {
                createJSONParticleRenderEditor();
            }
            particleRenderDataTextArea.classList.remove('multi-diff');
            setParticleRenderEditorJSON(particleRenderJSON);
            showSaveParticleRenderDataButton();
            hideParticleRenderDataTextArea();
            hideNewJSONParticleRenderEditorButton();
            hideParticleRenderDataSaved();
        } else {
            // normal text
            deleteJSONParticleRenderEditor();
            particleRenderDataTextArea.classList.toggle('multi-diff', particleRenderDataMultiValue.isMultiDiffValue);
            particleRenderDataTextArea.value = particleRenderDataMultiValue.isMultiDiffValue ? "" : particleRenderDataMultiValue.value;

            showParticleRenderDataTextArea();
            showNewJSONParticleRenderEditorButton();
            hideSaveParticleRenderDataButton();
            hideParticleRenderDataSaved();
        }

        let activeElement = document.activeElement;
        if (doSelectElement && typeof activeElement.select !== "undefined") {
            activeElement.select();
        }
    }
}

function loaded() {
    openEventBridge(function() {
        let elPropertiesList = document.getElementById("properties-pages");
        let elTabs = document.getElementById("tabs");

        GROUPS.forEach(function(group) {
            let elGroup;

            elGroup = document.createElement('div');
            elGroup.className = 'section ' + "major";
            elGroup.setAttribute("id", "properties-" + group.id);
            elPropertiesList.appendChild(elGroup);

            if (group.label !== undefined) {
                let elLegend = document.createElement('div');
                elLegend.className = "tab-section-header";
                elLegend.appendChild(createElementFromHTML(`<div class="labelTabHeader">${group.label}</div>`));
                elGroup.appendChild(elLegend);
                elTabs.appendChild(createElementFromHTML('<button id="tab-'+ group.id +'" onclick="showPage(' + "'" + group.id  + "'" + ');"><img src="tabs/'+ group.id +'.png"></button>'));
            }

            group.properties.forEach(function(propertyData) {
                let propertyType = propertyData.type;
                let propertyID = propertyData.propertyID;
                let propertyName = propertyData.propertyName !== undefined ? propertyData.propertyName : propertyID;
                let propertySpaceMode = propertyData.spaceMode !== undefined ? propertyData.spaceMode : PROPERTY_SPACE_MODE.ALL;
                let propertyElementID = "property-" + propertyID;
                propertyElementID = propertyElementID.replace('.', '-');

                let elContainer, elLabel;

                if (propertyData.replaceID === undefined) {
                    // Create subheader, or create new property and append it.
                    if (propertyType === "sub-header") {
                        elContainer = createElementFromHTML(
                            `<div class="sub-section-header legend">${propertyData.label}</div>`);
                    } else {
                        elContainer = document.createElement('div');
                        elContainer.setAttribute("id", "div-" + propertyElementID);
                        elContainer.className = 'property container';
                    }

                    if (group.twoColumn && propertyData.column !== undefined && propertyData.column !== -1) {
                        let columnName = group.id + "column" + propertyData.column;
                        let elColumn = document.getElementById(columnName);
                        if (!elColumn) {
                            let columnDivName = group.id + "columnDiv";
                            let elColumnDiv = document.getElementById(columnDivName);
                            if (!elColumnDiv) {
                                elColumnDiv = document.createElement('div');
                                elColumnDiv.className = "two-column";
                                elColumnDiv.setAttribute("id", group.id + "columnDiv");
                                elGroup.appendChild(elColumnDiv);
                            }
                            elColumn = document.createElement('fieldset');
                            elColumn.className = "column";
                            elColumn.setAttribute("id", columnName);
                            elColumnDiv.appendChild(elColumn);
                        }
                        elColumn.appendChild(elContainer);
                    } else {
                        elGroup.appendChild(elContainer);
                    }

                    let labelText = propertyData.label !== undefined ? propertyData.label : "";
                    let className = '';
                    if (propertyData.indentedLabel || propertyData.showPropertyRule !== undefined) {
                        className = 'indented';
                    }
                    elLabel = createElementFromHTML(
                        `<label><span class="${className}">${labelText}</span></label>`);
                    elContainer.appendChild(elLabel);
                } else {
                    elContainer = document.getElementById(propertyData.replaceID);
                }

                if (elLabel) {
                    createAppTooltip.registerTooltipElement(elLabel.childNodes[0], propertyID, propertyName);
                }

                let elProperty = createElementFromHTML('<div style="width: 100%;"></div>');
                elContainer.appendChild(elProperty);

                if (propertyType === 'triple') {
                    elProperty.className = 'flex-row';
                    for (let i = 0; i < propertyData.properties.length; ++i) {
                        let innerPropertyData = propertyData.properties[i];

                        let elWrapper = createElementFromHTML('<div class="triple-item"></div>');
                        elProperty.appendChild(elWrapper);

                        let propertyID = innerPropertyData.propertyID;               
                        let propertyName = innerPropertyData.propertyName !== undefined ? innerPropertyData.propertyName : propertyID;
                        let propertyElementID = "property-" + propertyID;
                        propertyElementID = propertyElementID.replace('.', '-');

                        let property = createProperty(innerPropertyData, propertyElementID, propertyName, propertyID, elWrapper);
                        property.isParticleProperty = group.id.includes("particles");
                        property.elContainer = elContainer;
                        property.spaceMode = propertySpaceMode;
                        property.group_id = group.id;

                        let elLabel = createElementFromHTML(`<div class="triple-label">${innerPropertyData.label}</div>`);
                        createAppTooltip.registerTooltipElement(elLabel, propertyID, propertyName);

                        elWrapper.appendChild(elLabel);
                        
                        if (property.type !== 'placeholder') {
                            properties[propertyID] = property;
                        }
                        if (innerPropertyData.type === 'number' || innerPropertyData.type === 'number-draggable') {
                            propertyRangeRequests.push(propertyID);
                        }
                    }
                } else {
                    let property = createProperty(propertyData, propertyElementID, propertyName, propertyID, elProperty);
                    property.isParticleProperty = group.id.includes("particles");
                    property.elContainer = elContainer;
                    property.spaceMode = propertySpaceMode;
                    property.group_id = group.id;

                    if (property.type !== 'placeholder') {
                        properties[propertyID] = property;
                    }
                    if (propertyData.type === 'number' || propertyData.type === 'number-draggable' || 
                        propertyData.type === 'vec2' || propertyData.type === 'vec3' ||
                        propertyData.type === 'rect' || propertyData.type === 'vec3rgb') {
                        propertyRangeRequests.push(propertyID);
                    }

                    let showPropertyRule = propertyData.showPropertyRule;
                    if (showPropertyRule !== undefined) {
                        let dependentProperty = Object.keys(showPropertyRule)[0];
                        let dependentPropertyValue = showPropertyRule[dependentProperty];
                        if (properties[dependentProperty] === undefined) {
                            properties[dependentProperty] = {};
                        }
                        if (properties[dependentProperty].showPropertyRules === undefined) {
                            properties[dependentProperty].showPropertyRules = {};
                        }
                        properties[dependentProperty].showPropertyRules[propertyID] = dependentPropertyValue;
                    }
                }
            });

            elGroups[group.id] = elGroup;
        });

        updateVisibleSpaceModeProperties();
        requestZoneList();

        if (window.EventBridge !== undefined) {
            EventBridge.scriptEventReceived.connect(function(data) {
                data = JSON.parse(data);
                if (data.type === "server_script_status" && selectedEntityIDs.size === 1) {
                    let elServerScriptError = document.getElementById("property-serverScripts-error");
                    let elServerScriptStatus = document.getElementById("property-serverScripts-status");
                    elServerScriptError.value = data.errorInfo;
                    // If we just set elServerScriptError's display to block or none, we still end up with
                    // it's parent contributing 21px bottom padding even when elServerScriptError is display:none.
                    // So set it's parent to block or none
                    elServerScriptError.parentElement.style.display = data.errorInfo ? "block" : "none";
                    if (data.statusRetrieved === false) {
                        elServerScriptStatus.innerText = "Failed to retrieve status";
                    } else if (data.isRunning) {
                        elServerScriptStatus.innerText = ENTITY_SCRIPT_STATUS[data.status] || data.status;
                    } else {
                        elServerScriptStatus.innerText = NOT_RUNNING_SCRIPT_STATUS;
                    }
                } else if (data.type === "update" && data.selections) {
                    if (data.spaceMode !== undefined) {
                        currentSpaceMode = data.spaceMode === "local" ? PROPERTY_SPACE_MODE.LOCAL : PROPERTY_SPACE_MODE.WORLD;
                    }
                    handleEntitySelectionUpdate(data.selections, data.isPropertiesToolUpdate);
                } else if (data.type === 'tooltipsReply') {
                    createAppTooltip.setIsEnabled(!data.hmdActive);
                    createAppTooltip.setTooltipData(data.tooltips);
                } else if (data.type === 'hmdActiveChanged') {
                    createAppTooltip.setIsEnabled(!data.hmdActive);
                } else if (data.type === 'setSpaceMode') {
                    currentSpaceMode = data.spaceMode === "local" ? PROPERTY_SPACE_MODE.LOCAL : PROPERTY_SPACE_MODE.WORLD;
                    updateVisibleSpaceModeProperties();
                } else if (data.type === 'propertyRangeReply') {
                    let propertyRanges = data.propertyRanges;
                    for (let property in propertyRanges) {
                        let propertyRange = propertyRanges[property];
                        if (propertyRange !== undefined) {
                            let propertyData = properties[property].data;
                            let multiplier = propertyData.multiplier;
                            if (propertyData.min === undefined && propertyRange.minimum !== "") {
                                propertyData.min = propertyRange.minimum;
                                if (multiplier !== undefined) {
                                    propertyData.min /= multiplier;
                                }
                            }
                            if (propertyData.max === undefined && propertyRange.maximum !== "") {
                                propertyData.max = propertyRange.maximum;
                                if (multiplier !== undefined) {
                                    propertyData.max /= multiplier;
                                }
                            }
                            switch (propertyData.type) {
                                case 'number':
                                    updateNumberMinMax(properties[property]);
                                    break;
                                case 'number-draggable':
                                    updateNumberDraggableMinMax(properties[property]);
                                    break;
                                case 'vec3':
                                case 'vec2':
                                    updateVectorMinMax(properties[property]);
                                    break;
                                case 'vec3rgb':
                                    updateVectorMinMax(properties[property]);
                                    break;
                                case 'rect':
                                    updateRectMinMax(properties[property]);
                                    break;
                            }
                        }
                    }
                } else if (data.type === 'materialTargetReply') {
                    if (data.entityID === getFirstSelectedID()) {
                        setMaterialTargetData(data.materialTargetData);
                    }
                } else if (data.type === 'zoneListRequest') {
                    zonesList = data.zones;
                } else if (data.type === 'urlPermissionChanged') {
                    canViewAssetURLs = data.canViewAssetURLs;
                    Object.entries(properties).forEach(function ([propertyID, property]) {
                        if (property.data.placeholder && property.data.placeholder === "URL") {
                            if (!canViewAssetURLs) {
                                property.elInput.value = "";
                            }
                            property.elInput.placeholder = canViewAssetURLs ? property.data.placeholder : "You don't have permission to view this URL";
                        }
                    });
                }
            });

            // Request tooltips and property ranges as soon as we can process a reply:
            EventBridge.emitWebEvent(JSON.stringify({ type: 'tooltipsRequest' }));
            EventBridge.emitWebEvent(JSON.stringify({ type: 'propertyRangeRequest', properties: propertyRangeRequests }));
        }

        // Server Script Status
        let elServerScriptStatusOuter = document.getElementById('div-property-serverScriptStatus');
        let elServerScriptStatusContainer = document.getElementById('div-property-serverScriptStatus').childNodes[1];
        let serverScriptStatusElementID = 'property-serverScripts-status';
        createAppTooltip.registerTooltipElement(elServerScriptStatusOuter.childNodes[0], "serverScriptsStatus");
        let elServerScriptStatus = document.createElement('div');
        elServerScriptStatus.setAttribute("id", serverScriptStatusElementID);
        elServerScriptStatusContainer.appendChild(elServerScriptStatus);

        // Server Script Error
        let elServerScripts = getPropertyInputElement("serverScripts");
        let elDiv = document.createElement('div');
        elDiv.className = "property";
        let elServerScriptError = document.createElement('textarea');
        let serverScriptErrorElementID = 'property-serverScripts-error';
        elServerScriptError.setAttribute("id", serverScriptErrorElementID);
        elDiv.appendChild(elServerScriptError);
        elServerScriptStatusContainer.appendChild(elDiv);

        let elScript = getPropertyInputElement("script");
        elScript.parentNode.className = "url refresh";
        elServerScripts.parentNode.className = "url refresh";

        let elParentID = getPropertyInputElement("parentID");
        elParentID.parentNode.className = "url refresh";
        
        // User Data
        let userDataProperty = properties["userData"];
        let elUserData = userDataProperty.elInput;
        let userDataElementID = userDataProperty.elementID;
        elDiv = elUserData.parentNode;
        let elUserDataEditor = document.createElement('div');
        elUserDataEditor.setAttribute("id", userDataElementID + "-editor");
        let elUserDataEditorStatus = document.createElement('div');
        elUserDataEditorStatus.setAttribute("id", userDataElementID + "-editorStatus");
        let elUserDataSaved = document.createElement('span');
        elUserDataSaved.setAttribute("id", userDataElementID + "-saved");
        elUserDataSaved.innerText = "Saved!";
        elDiv.childNodes[JSON_EDITOR_ROW_DIV_INDEX].appendChild(elUserDataSaved);
        elDiv.insertBefore(elUserDataEditor, elUserData);
        elDiv.insertBefore(elUserDataEditorStatus, elUserData);

        // Material Data
        let materialDataProperty = properties["materialData"];
        let elMaterialData = materialDataProperty.elInput;
        let materialDataElementID = materialDataProperty.elementID;
        elDiv = elMaterialData.parentNode;
        let elMaterialDataEditor = document.createElement('div');
        elMaterialDataEditor.setAttribute("id", materialDataElementID + "-editor");
        let elMaterialDataEditorStatus = document.createElement('div');
        elMaterialDataEditorStatus.setAttribute("id", materialDataElementID + "-editorStatus");
        let elMaterialDataSaved = document.createElement('span');
        elMaterialDataSaved.setAttribute("id", materialDataElementID + "-saved");
        elMaterialDataSaved.innerText = "Saved!";
        elDiv.childNodes[JSON_EDITOR_ROW_DIV_INDEX].appendChild(elMaterialDataSaved);
        elDiv.insertBefore(elMaterialDataEditor, elMaterialData);
        elDiv.insertBefore(elMaterialDataEditorStatus, elMaterialData);

        // Particle Update + Render Data
        let particleUpdateDataProperty = properties["particleUpdateData"];
        let elParticleUpdateData = particleUpdateDataProperty.elInput;
        let particleUpdateDataElementID = particleUpdateDataProperty.elementID;
        elDiv = elParticleUpdateData.parentNode;
        let elParticleUpdateDataEditor = document.createElement('div');
        elParticleUpdateDataEditor.setAttribute("id", particleUpdateDataElementID + "-editor");
        let elParticleUpdateDataEditorStatus = document.createElement('div');
        elParticleUpdateDataEditorStatus.setAttribute("id", particleUpdateDataElementID + "-editorStatus");
        let elParticleUpdateDataSaved = document.createElement('span');
        elParticleUpdateDataSaved.setAttribute("id", particleUpdateDataElementID + "-saved");
        elParticleUpdateDataSaved.innerText = "Saved!";
        elDiv.childNodes[JSON_EDITOR_ROW_DIV_INDEX].appendChild(elParticleUpdateDataSaved);
        elDiv.insertBefore(elParticleUpdateDataEditor, elParticleUpdateData);
        elDiv.insertBefore(elParticleUpdateDataEditorStatus, elParticleUpdateData);

        let particleRenderDataProperty = properties["particleRenderData"];
        let elParticleRenderData = particleRenderDataProperty.elInput;
        let particleRenderDataElementID = particleRenderDataProperty.elementID;
        elDiv = elParticleRenderData.parentNode;
        let elParticleRenderDataEditor = document.createElement('div');
        elParticleRenderDataEditor.setAttribute("id", particleRenderDataElementID + "-editor");
        let elParticleRenderDataEditorStatus = document.createElement('div');
        elParticleRenderDataEditorStatus.setAttribute("id", particleRenderDataElementID + "-editorStatus");
        let elParticleRenderDataSaved = document.createElement('span');
        elParticleRenderDataSaved.setAttribute("id", particleRenderDataElementID + "-saved");
        elParticleRenderDataSaved.innerText = "Saved!";
        elDiv.childNodes[JSON_EDITOR_ROW_DIV_INDEX].appendChild(elParticleRenderDataSaved);
        elDiv.insertBefore(elParticleRenderDataEditor, elParticleRenderData);
        elDiv.insertBefore(elParticleRenderDataEditorStatus, elParticleRenderData);

        // Textarea scrollbars
        let elTextareas = document.getElementsByTagName("TEXTAREA");

        let textareaOnChangeEvent = function(event) {
            setTextareaScrolling(event.target);
        };

        for (let textAreaIndex = 0, numTextAreas = elTextareas.length; textAreaIndex < numTextAreas; ++textAreaIndex) {
            let curTextAreaElement = elTextareas[textAreaIndex];
            setTextareaScrolling(curTextAreaElement);
            curTextAreaElement.addEventListener("input", textareaOnChangeEvent, false);
            curTextAreaElement.addEventListener("change", textareaOnChangeEvent, false);
            /* FIXME: Detect and update textarea scrolling attribute on resize. Unfortunately textarea doesn't have a resize
            event; mouseup is a partial stand-in but doesn't handle resizing if mouse moves outside textarea rectangle. */
            curTextAreaElement.addEventListener("mouseup", textareaOnChangeEvent, false);
        }

        // Dropdowns
        // For each dropdown the following replacement is created in place of the original dropdown...
        // Structure created:
        //  <dl dropped="true/false">
        //      <dt name="?" id="?" value="?"><span>display text</span><span>carat</span></dt>
        //      <dd>
        //          <ul>
        //              <li value="??>display text</li>
        //              <li>...</li>
        //          </ul>
        //      </dd>
        //  </dl>
        let elDropdowns = document.getElementsByTagName("select");
        for (let dropDownIndex = 0; dropDownIndex < elDropdowns.length; ++dropDownIndex) {
            let elDropdown = elDropdowns[dropDownIndex];
            let options = elDropdown.getElementsByTagName("option");
            let selectedOption = 0;
            for (let optionIndex = 0; optionIndex < options.length; ++optionIndex) {
                if (options[optionIndex].getAttribute("selected") === "selected") {
                    selectedOption = optionIndex;
                    break;
                }
            }
            let div = elDropdown.parentNode;

            let dl = document.createElement("dl");
            div.appendChild(dl);

            let dt = document.createElement("dt");
            dt.name = elDropdown.name;
            dt.id = elDropdown.id;
            dt.addEventListener("click", toggleDropdown, true);
            dl.appendChild(dt);

            let elMultiDiff = document.createElement('span');
            elMultiDiff.className = "multi-diff";
            dl.appendChild(elMultiDiff);

            let span = document.createElement("span");
            span.setAttribute("value", options[selectedOption].value);
            span.textContent = options[selectedOption].firstChild.textContent;
            dt.appendChild(span);

            let spanCaratDown = document.createElement("span");
            spanCaratDown.textContent = "5"; // caratDn
            dt.appendChild(spanCaratDown);

            let dd = document.createElement("dd");
            dl.appendChild(dd);

            let ul = document.createElement("ul");
            dd.appendChild(ul);

            for (let listOptionIndex = 0; listOptionIndex < options.length; ++listOptionIndex) {
                let li = document.createElement("li");
                li.setAttribute("value", options[listOptionIndex].value);
                li.textContent = options[listOptionIndex].firstChild.textContent;
                li.addEventListener("click", setDropdownValue);
                ul.appendChild(li);
            }

            let propertyID = elDropdown.getAttribute("propertyID");
            let property = properties[propertyID];
            property.elInput = dt;
            
            if (property.data.
                skipPropertyUpdate !== true) {
                    dt.addEventListener('change', createEmitTextPropertyUpdateFunction(property));
            }
            
            if (property.data.
                onDropdownChange !== undefined) {
                dt.addEventListener('change', property.data.onDropdownChange(property));
            }
        }

        document.addEventListener('click', function(ev) { closeAllDropdowns() }, true);
        
        elDropdowns = document.getElementsByTagName("select");
        while (elDropdowns.length > 0) {
            let el = elDropdowns[0];
            el.parentNode.removeChild(el);
            elDropdowns = document.getElementsByTagName("select");
        }

        const KEY_CODES = {
            BACKSPACE: 8,
            DELETE: 46
        };

        document.addEventListener("keyup", function (keyUpEvent) {
            const FILTERED_NODE_NAMES = ["INPUT", "TEXTAREA"];
            if (FILTERED_NODE_NAMES.includes(keyUpEvent.target.nodeName)) {
                return;
            }

            if (elUserDataEditor.contains(keyUpEvent.target) || elMaterialDataEditor.contains(keyUpEvent.target) || elParticleUpdateDataEditor.contains(keyUpEvent.target)
                || elParticleRenderDataEditor.contains(keyUpEvent.target)) {
                return;
            }

            let {code, key, keyCode, altKey, ctrlKey, metaKey, shiftKey} = keyUpEvent;

            let controlKey = window.navigator.platform.startsWith("Mac") ? metaKey : ctrlKey;

            let keyCodeString;
            switch (keyCode) {
                case KEY_CODES.DELETE:
                    keyCodeString = "Delete";
                    break;
                case KEY_CODES.BACKSPACE:
                    keyCodeString = "Backspace";
                    break;
                default:
                    keyCodeString = String.fromCharCode(keyUpEvent.keyCode);
                    break;
            }

            EventBridge.emitWebEvent(JSON.stringify({
                type: 'keyUpEvent',
                keyUpEvent: {
                    code,
                    key,
                    keyCode,
                    keyCodeString,
                    altKey,
                    controlKey,
                    shiftKey,
                }
            }));
        }, false);

        window.onblur = function() {
            // Fake a change event
            let ev = document.createEvent("HTMLEvents");
            ev.initEvent("change", true, true);
            document.activeElement.dispatchEvent(ev);
        };

        // For input and textarea elements, select all of the text on focus
        let els = document.querySelectorAll("input, textarea");
        for (let i = 0; i < els.length; ++i) {
            els[i].onfocus = function (e) {
                e.target.select();
            };
        }

        bindAllNonJSONEditorElements();

        showGroupsForType("None");
        showPage("base");
        resetProperties();
        disableProperties();

    });

    augmentSpinButtons();
    disableDragDrop();

    // Disable right-click context menu which is not visible in the HMD and makes it seem like the app has locked
    document.addEventListener("contextmenu", function(event) {
        event.preventDefault();
    }, false);

    setTimeout(function() {
        EventBridge.emitWebEvent(JSON.stringify({ type: 'propertiesPageReady' }));
    }, 1000);
}

function showOnTheSamePage(entityType) {
    let numberOfTypes = entityType.length;
    let matchingType = 0;
    for (let i = 0; i < numberOfTypes; i++) {
        if (GROUPS_PER_TYPE[entityType[i]].includes(currentTab)) {
            matchingType = matchingType + 1;
        }
    }
    if (matchingType !== numberOfTypes) {
        currentTab = "base";
    }
    showPage(currentTab);
}

function showPage(id) {
    currentTab = id;
    Object.entries(elGroups).forEach(([groupKey, elGroup]) => {
        if (groupKey === id) {
            elGroup.style.display = "block";
            document.getElementById("tab-" + groupKey).style.backgroundColor = "#2E2E2E";
        } else {
            elGroup.style.display = "none";
            document.getElementById("tab-" + groupKey).style.backgroundColor = "#404040";
        }
    });
}
