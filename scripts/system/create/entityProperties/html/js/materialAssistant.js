//
//  materialAssistant.js
//
//  Created by Alezia Kurdis on May 19th, 2022.
//  Copyright 2022 Vircadia contributors.
//  Copyright 2022-2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

let maMaterialData = {};

let maClose,
    maName,
    maAlbedoIsActive,
    maAlbedoColorPicker,
    maAlbedoColorPickerSel,
    maAlbedoMapIsActive,
    maAlbedoMap,
    maMetallicIsActive,
    maMetallic,
    maMetallicSlider,
    maMetallicMap,
    maUseSpecular,
    maRoughnessIsActive,
    maRoughness,
    maRoughnessSlider,
    maRoughnessMap,
    maUseGloss,
    maNormalMapIsActive,
    maNormalMap,
    maUseBump,
    maOpacityIsActive,
    maOpacity,
    maOpacitySlider,
    maOpacityMapModeDont,
    maOpacityMapModeMask,
    maOpacityMapModeBlend,
    maOpacityCutoff,
    maOpacityCutoffSlider,
    maEmissiveIsActive,
    maEmissiveColorPicker,
    maEmissiveColorPickerSel,
    maBloom,
    maBloomSlider,
    maUnlit,
    maEmissiveMap,
    maScatteringIsActive,
    maScattering,
    maScatteringSlider,
    maScatteringMap,
    maOcclusionMapIsActive,
    maOcclusionMap,
    maLightMapIsActive,
    maLightMap,
    maShadeIsActive,
    maShadeColorPicker,
    maShadeColorPickerSel,
    maShadeMapIsActive,
    maShadeMap,
    maMatcapIsActive,
    maMatcapColorPicker,
    maMatcapColorPickerSel,
    maMatcapMapIsActive,
    maMatcapMap,
    maShadingShiftIsActive,
    maShadingShift,
    maShadingShiftSlider,
    maShadingShiftMapIsActive,
    maShadingShiftMap,
    maShadingToonyIsActive,
    maShadingToony,
    maShadingToonySlider,
    maParametricRimIsActive,
    maParametricRimColorPicker,
    maParametricRimColorPickerSel,
    maRimMapIsActive,
    maRimMap,
    maParametricRimFresnelPowerIsActive,
    maParametricRimFresnelPower,
    maParametricRimFresnelPowerSlider,
    maParametricRimLiftIsActive,
    maParametricRimLift,
    maParametricRimLiftSlider,
    maRimLightingMixIsActive,
    maRimLightingMix,
    maRimLightingMixSlider,
/* ############## Not yet supported, but the code is ready (commented) ###################
    maOutlineWidthModeIsActive,
    maOutlineWidthModeNone,
    maOutlineWidthModeWorld,
    maOutlineWidthModeScreen,
    maOutlineWidthIsActive,
    maOutlineWidth,
    maOutlineWidthSlider,
    maOutlineIsActive,
    maOutlineColorPicker,
    maOutlineColorPickerSel,
*/  
    maUvAnimationMaskMapIsActive,
    maUvAnimationMaskMap,
    maUvAnimationScrollXSpeedIsActive,
    maUvAnimationScrollXSpeed,
    maUvAnimationScrollXSpeedSlider,
    maUvAnimationScrollYSpeedIsActive,
    maUvAnimationScrollYSpeed,
    maUvAnimationScrollYSpeedSlider,
    maUvAnimationRotationSpeedIsActive,
    maUvAnimationRotationSpeed,
    maUvAnimationRotationSpeedSlider,
    maProcedural,
    maAddProceduralTemplate,
    maCullFaceModeBack,
    maCullFaceModeFront,
    maCullFaceModeNone,
    maModelHifiPbr,
    maModelVrmMtoon,
    maModelHifiShaderSimple;

let DEFAULT_ALBEDO = [1,1,1];
let DEFAULT_EMISSIVE = [0,0,0];
let DEFAULT_BLOOM_FACTOR = 1;
let DEFAULT_ROUGHNESS = 0.9;
let DEFAULT_METALLIC_FOR_MA_UI = 0.001;
let DEFAULT_METALLIC = 0;
let DEFAULT_OPACITY = 1;
let DEFAULT_OPACITY_CUTOFF = 0.5;
let DEFAULT_SCATTERING = 0;
let DEFAULT_MODEL = "hifi_pbr";
let DEFAULT_SHADE = [1,1,1];
let DEFAULT_MATCAP = [1,1,1];
let DEFAULT_SHADING_SHIFT = 0.0;
let DEFAULT_SHADING_TOONY = 0.9;
let DEFAULT_PARAMETRIC_RIM = [0,0,0];
let DEFAULT_RIM_LIGHTING_MIX = 1.0;
let DEFAULT_PARAMETRIC_RIM_FRESNEL_POWER = 5.0;
let DEFAULT_PARAMETRIC_RIM_LIFT = 0.0;

/* ############## Not yet supported, but the code is ready ###################
let DEFAULT_OUTLINE = [0,0,0];
let DEFAULT_OUTLINE_WIDTH_MODE = "none";
let DEFAULT_OUTLINE_WIDTH = 0.0;
//#################################################################
*/
let DEFAULT_UV_ANIMATION_SCROLL_X_SPEED = 0.0;
let DEFAULT_UV_ANIMATION_SCROLL_Y_SPEED = 0.0;
let DEFAULT_UV_ANIMATION_ROTATION_SPEED = 0.0;

function initiateMaUi() {
    maClose = document.getElementById("uiMaterialAssistant-closeButton");
    maName = document.getElementById("ma-name");
    maAlbedoIsActive = document.getElementById("ma-albedo-isActive");
    maAlbedoColorPicker = document.getElementById("ma-albedo-colorPicker");
    maAlbedoMapIsActive = document.getElementById("ma-albedoMap-isActive");
    maAlbedoMap = document.getElementById("ma-albedoMap");
    maMetallicIsActive = document.getElementById("ma-metallic-isActive");
    maMetallic = document.getElementById("ma-metallic");
    maMetallicSlider = document.getElementById("ma-metallic-slider");
    maMetallicMap = document.getElementById("ma-metallicMap");
    maUseSpecular = document.getElementById("ma-useSpecular");
    maRoughnessIsActive = document.getElementById("ma-roughness-isActive");
    maRoughness = document.getElementById("ma-roughness");
    maRoughnessSlider = document.getElementById("ma-roughness-slider");
    maRoughnessMap = document.getElementById("ma-roughnessMap");
    maUseGloss = document.getElementById("ma-useGloss");
    maNormalMapIsActive = document.getElementById("ma-normalMap-isActive");
    maNormalMap = document.getElementById("ma-normalMap");
    maUseBump = document.getElementById("ma-useBump");
    maOpacityIsActive = document.getElementById("ma-opacity-isActive");
    maOpacity = document.getElementById("ma-opacity");
    maOpacitySlider = document.getElementById("ma-opacity-slider");
    maOpacityMapModeDont = document.getElementById("ma-opacityMapMode-dont");
    maOpacityMapModeMask = document.getElementById("ma-opacityMapMode-mask");
    maOpacityMapModeBlend = document.getElementById("ma-opacityMapMode-blend");
    maOpacityCutoff = document.getElementById("ma-opacityCutoff");
    maOpacityCutoffSlider = document.getElementById("ma-opacityCutoff-slider");
    maEmissiveIsActive = document.getElementById("ma-emissive-isActive");
    maEmissiveColorPicker = document.getElementById("ma-emissive-colorPicker");
    maBloom = document.getElementById("ma-bloom");
    maBloomSlider = document.getElementById("ma-bloom-slider");
    maUnlit = document.getElementById("ma-unlit");
    maEmissiveMap = document.getElementById("ma-emissiveMap");
    maScatteringIsActive = document.getElementById("ma-scattering-isActive");
    maScattering = document.getElementById("ma-scattering");
    maScatteringSlider = document.getElementById("ma-scattering-slider");
    maScatteringMap = document.getElementById("ma-scatteringMap");
    maOcclusionMapIsActive = document.getElementById("ma-occlusionMap-isActive");
    maOcclusionMap = document.getElementById("ma-occlusionMap");
    maLightMapIsActive = document.getElementById("ma-lightMap-isActive");
    maLightMap = document.getElementById("ma-lightMap");
    maShadeIsActive = document.getElementById("ma-shade-isActive");
    maShadeColorPicker = document.getElementById("ma-shade-colorPicker");
    maShadeMapIsActive = document.getElementById("ma-shadeMap-isActive");
    maShadeMap = document.getElementById("ma-shadeMap");
    maMatcapIsActive = document.getElementById("ma-matcap-isActive");
    maMatcapColorPicker = document.getElementById("ma-matcap-colorPicker");
    maMatcapMapIsActive = document.getElementById("ma-matcapMap-isActive");
    maMatcapMap = document.getElementById("ma-matcapMap");
    maShadingShiftIsActive = document.getElementById("ma-shadingShift-isActive");
    maShadingShift = document.getElementById("ma-shadingShift");
    maShadingShiftSlider = document.getElementById("ma-shadingShift-slider");
    maShadingShiftMapIsActive = document.getElementById("ma-shadingShiftMap-isActive");
    maShadingShiftMap = document.getElementById("ma-shadingShiftMap");
    maShadingToonyIsActive = document.getElementById("ma-shadingToony-isActive");
    maShadingToony = document.getElementById("ma-shadingToony");
    maShadingToonySlider = document.getElementById("ma-shadingToony-slider");
    maParametricRimIsActive = document.getElementById("ma-parametricRim-isActive");
    maParametricRimColorPicker = document.getElementById("ma-parametricRim-colorPicker");
    maRimMapIsActive = document.getElementById("ma-rimMap-isActive");
    maRimMap = document.getElementById("ma-rimMap");
    maParametricRimFresnelPowerIsActive = document.getElementById("ma-parametricRimFresnelPower-isActive");
    maParametricRimFresnelPower = document.getElementById("ma-parametricRimFresnelPower");
    maParametricRimFresnelPowerSlider = document.getElementById("ma-parametricRimFresnelPower-slider");
    maParametricRimLiftIsActive = document.getElementById("ma-parametricRimLift-isActive");
    maParametricRimLift = document.getElementById("ma-parametricRimLift");
    maParametricRimLiftSlider = document.getElementById("ma-parametricRimLift-slider");
    maRimLightingMixIsActive = document.getElementById("ma-rimLightingMix-isActive");
    maRimLightingMix = document.getElementById("ma-rimLightingMix");
    maRimLightingMixSlider = document.getElementById("ma-rimLightingMix-slider");
    
/* ############## Not yet supported, but the code is ready ################################
    maOutlineWidthModeIsActive = document.getElementById("ma-outlineWidthMode-isActive");
    maOutlineWidthModeNone = document.getElementById("ma-outlineWidthMode-none");
    maOutlineWidthModeWorld = document.getElementById("ma-outlineWidthMode-world");
    maOutlineWidthModeScreen = document.getElementById("ma-outlineWidthMode-screen");
    maOutlineWidthIsActive = document.getElementById("ma-outlineWidth-isActive");
    maOutlineWidth = document.getElementById("ma-outlineWidth");
    maOutlineWidthSlider = document.getElementById("ma-outlineWidth-slider");
    maOutlineIsActive = document.getElementById("ma-outline-isActive");
    maOutlineColorPicker = document.getElementById("ma-outline-colorPicker");
    //#################################################################################
*/
    maUvAnimationMaskMapIsActive = document.getElementById("ma-uvAnimationMaskMap-isActive");
    maUvAnimationMaskMap = document.getElementById("ma-uvAnimationMaskMap");
    maUvAnimationScrollXSpeedIsActive = document.getElementById("ma-uvAnimationScrollXSpeed-isActive");
    maUvAnimationScrollXSpeed = document.getElementById("ma-uvAnimationScrollXSpeed");
    maUvAnimationScrollXSpeedSlider = document.getElementById("ma-uvAnimationScrollXSpeed-slider");
    maUvAnimationScrollYSpeedIsActive = document.getElementById("ma-uvAnimationScrollYSpeed-isActive");
    maUvAnimationScrollYSpeed = document.getElementById("ma-uvAnimationScrollYSpeed");
    maUvAnimationScrollYSpeedSlider = document.getElementById("ma-uvAnimationScrollYSpeed-slider");
    maUvAnimationRotationSpeedIsActive = document.getElementById("ma-uvAnimationRotationSpeed-isActive");
    maUvAnimationRotationSpeed = document.getElementById("ma-uvAnimationRotationSpeed");
    maUvAnimationRotationSpeedSlider = document.getElementById("ma-uvAnimationRotationSpeed-slider");
    maProcedural = document.getElementById("ma-procedural");
    maAddProceduralTemplate = document.getElementById("ma-addProceduralTemplate");
    maCullFaceModeBack = document.getElementById("ma-cullFaceMode-back");
    maCullFaceModeFront = document.getElementById("ma-cullFaceMode-front");
    maCullFaceModeNone = document.getElementById("ma-cullFaceMode-none");
    maModelHifiPbr = document.getElementById("ma-Model-hifi_pbr");
    maModelVrmMtoon = document.getElementById("ma-Model-vrm_mtoon");
    maModelHifiShaderSimple = document.getElementById("ma-Model-hifi_shader_simple");
    
    maClose.onclick = function() {
        closeMaterialAssistant();
    };
    maAlbedoIsActive.onclick = function() {
        if (maMaterialData.albedoIsActive) {
            maMaterialData.albedoIsActive = false;
            maAlbedoIsActive.className = "uiMaterialAssistant-inactive";
            maAlbedoColorPicker.style.pointerEvents = 'none';
            maAlbedoColorPicker.style.backgroundColor = "#555555";
        } else {
            maMaterialData.albedoIsActive = true;
            maAlbedoIsActive.className = "uiMaterialAssistant-active";
            maAlbedoColorPicker.style.pointerEvents = 'auto';
            maAlbedoColorPicker.style.backgroundColor = maGetRGB(maMaterialData.albedo);
        }
        maGenerateJsonAndSave();
    };
    maAlbedoMapIsActive.onclick = function() {
        if (maMaterialData.albedoMapIsActive) {
            maMaterialData.albedoMapIsActive = false;
            maAlbedoMapIsActive.className = "uiMaterialAssistant-inactive";
            maAlbedoMap.disabled = true;
        } else {
            maMaterialData.albedoMapIsActive = true;
            maAlbedoMapIsActive.className = "uiMaterialAssistant-active";
            maAlbedoMap.disabled = false;
            if (maMaterialData.albedoMap === undefined) {
                maMaterialData.albedoMap = "";
            }
        }
        maGenerateJsonAndSave();
    };
    maMetallicIsActive.onclick = function() {
        if (maMaterialData.metallicIsActive) {
            maMaterialData.metallicIsActive = false;
            maMetallicIsActive.className = "uiMaterialAssistant-inactive";
            maMetallic.disabled = true;
            maMetallicSlider.disabled = true;
            maMetallicMap.disabled = true;
            maUseSpecular.disabled = true;
        } else {
            maMaterialData.metallicIsActive = true;
            maMetallicIsActive.className = "uiMaterialAssistant-active";
            maMetallic.disabled = false;
            maMetallicSlider.disabled = false;
            maMetallicMap.disabled = false;
            maUseSpecular.disabled = false;
        }
        maGenerateJsonAndSave();  
    };
    maRoughnessIsActive.onclick = function() {
        if (maMaterialData.roughnessIsActive) {
            maMaterialData.roughnessIsActive = false;
            maRoughnessIsActive.className = "uiMaterialAssistant-inactive";
            maRoughness.disabled = true;
            maRoughnessSlider.disabled = true;
            maRoughnessMap.disabled = true;
            maUseGloss.disabled = true;
        } else {
            maMaterialData.roughnessIsActive = true;
            maRoughnessIsActive.className = "uiMaterialAssistant-active";
            maRoughness.disabled = false;
            maRoughnessSlider.disabled = false;
            maRoughnessMap.disabled = false;
            maUseGloss.disabled = false;
        }
        maGenerateJsonAndSave();  
    };
    maNormalMapIsActive.onclick = function() {
        if (maMaterialData.normalMapIsActive) {
            maMaterialData.normalMapIsActive = false;
            maNormalMapIsActive.className = "uiMaterialAssistant-inactive";
            maNormalMap.disabled = true;
            maUseBump.disabled = true;
        } else {
            maMaterialData.normalMapIsActive = true;
            maNormalMapIsActive.className = "uiMaterialAssistant-active";
            maNormalMap.disabled = false;
            maUseBump.disabled = false;
            if (maMaterialData.normalMap === undefined) {
                maMaterialData.normalMap = "";
            }
        }
        maGenerateJsonAndSave();
    };
    maOpacityIsActive.onclick = function() {
        if (maMaterialData.opacityIsActive) {
            maMaterialData.opacityIsActive = false;
            maOpacityIsActive.className = "uiMaterialAssistant-inactive";
            maOpacity.disabled = true;
            maOpacitySlider.disabled = true;
            maOpacityMapModeDont.disabled = true;
            maOpacityMapModeMask.disabled = true;
            maOpacityMapModeBlend.disabled = true;
            maOpacityCutoff.disabled = true;
            maOpacityCutoffSlider.disabled = true;
        } else {
            maMaterialData.opacityIsActive = true;
            maOpacityIsActive.className = "uiMaterialAssistant-active";
            maOpacity.disabled = false;
            maOpacitySlider.disabled = false;
            maOpacityMapModeDont.disabled = false;
            maOpacityMapModeMask.disabled = false;
            maOpacityMapModeBlend.disabled = false;
            maOpacityCutoff.disabled = false;
            maOpacityCutoffSlider.disabled = false;
        }
        maGenerateJsonAndSave(); 
    }; 
    maEmissiveIsActive.onclick = function() {
        if (maMaterialData.emissiveIsActive) {
            maMaterialData.emissiveIsActive = false;
            maEmissiveIsActive.className = "uiMaterialAssistant-inactive";
            maBloom.disabled = true;
            maBloomSlider.disabled = true;
            maEmissiveMap.disabled = true;
            maEmissiveColorPicker.style.pointerEvents = 'none';
            maEmissiveColorPicker.style.backgroundColor = "#555555";
        } else {
            maMaterialData.emissiveIsActive = true;
            maEmissiveIsActive.className = "uiMaterialAssistant-active";
            maBloom.disabled = false;
            maBloomSlider.disabled = false;
            maEmissiveMap.disabled = false;
            maEmissiveColorPicker.style.pointerEvents = 'auto';
            maEmissiveColorPicker.style.backgroundColor = maGetRGB(maMaterialData.emissive);
        }
        maGenerateJsonAndSave(); 
    };
    maScatteringIsActive.onclick = function() {
        if (maMaterialData.scatteringIsActive) {
            maMaterialData.scatteringIsActive = false;
            maScatteringIsActive.className = "uiMaterialAssistant-inactive";
            maScattering.disabled = true;
            maScatteringSlider.disabled = true;
            maScatteringMap.disabled = true;
        } else {
            maMaterialData.scatteringIsActive = true;
            maScatteringIsActive.className = "uiMaterialAssistant-active";
            maScattering.disabled = false;
            maScatteringSlider.disabled = false;
            maScatteringMap.disabled = false;            
        }
        maGenerateJsonAndSave(); 
    }; 
    maOcclusionMapIsActive.onclick = function() {
        if (maMaterialData.occlusionMapIsActive) {
            maMaterialData.occlusionMapIsActive = false;
            maOcclusionMapIsActive.className = "uiMaterialAssistant-inactive";
            maOcclusionMap.disabled = true;
        } else {
            maMaterialData.occlusionMapIsActive = true;
            maOcclusionMapIsActive.className = "uiMaterialAssistant-active";
            maOcclusionMap.disabled = false;
            if (maMaterialData.occlusionMap === undefined) {
                maMaterialData.occlusionMap = "";
            }
        }
        maGenerateJsonAndSave();
    };
    maLightMapIsActive.onclick = function() {
        if (maMaterialData.lightMapIsActive) {
            maMaterialData.lightMapIsActive = false;
            maLightMapIsActive.className = "uiMaterialAssistant-inactive";
            maLightMap.disabled = true;
        } else {
            maMaterialData.lightMapIsActive = true;
            maLightMapIsActive.className = "uiMaterialAssistant-active";
            maLightMap.disabled = false;
            if (maMaterialData.lightMap === undefined) {
                maMaterialData.lightMap = "";
            }
        }
        maGenerateJsonAndSave();
    };    
    maName.oninput = function() {
        maMaterialData.name = maName.value;
        maGenerateJsonAndSave();        
    };
    maAlbedoMap.oninput = function() {
        maMaterialData.albedoMap = maAlbedoMap.value;
        maGenerateJsonAndSave();
    };
    maMetallicSlider.oninput = function() {
        maMetallic.value = maMetallicSlider.value/1000;
        maMaterialData.metallic = parseFloat(maMetallic.value);
        maGenerateJsonAndSave();
    };
    maMetallicMap.oninput = function() {
        maMaterialData.metallicMap = maMetallicMap.value;
        maGenerateJsonAndSave();  
    };
    maUseSpecular.oninput = function() {
        maMaterialData.useSpecular = maUseSpecular.checked;
        maGenerateJsonAndSave();
    };
    maRoughnessSlider.oninput = function() {
        maRoughness.value = maRoughnessSlider.value/1000;
        maMaterialData.roughness = parseFloat(maRoughness.value);
        maGenerateJsonAndSave();
    };
    maRoughnessMap.oninput = function() {
        maMaterialData.roughnessMap = maRoughnessMap.value;
        maGenerateJsonAndSave();  
    };
    maUseGloss.oninput = function() {
        maMaterialData.useGloss = maUseGloss.checked;
        maGenerateJsonAndSave();
    };
    maNormalMap.oninput = function() {
        maMaterialData.normalMap = maNormalMap.value;
        maGenerateJsonAndSave(); 
    };
    maUseBump.oninput = function() {
        maMaterialData.useBump = maUseBump.checked;
        maGenerateJsonAndSave();
    };
    maOpacitySlider.oninput = function() {
        maOpacity.value = maOpacitySlider.value/1000;
        maMaterialData.opacity = parseFloat(maOpacity.value);
        maGenerateJsonAndSave();
    };
    maOpacityMapModeDont.oninput = function() {
        maMaterialData.opacityMapMode = maOpacityMapModeDont.value;
        maGenerateJsonAndSave();        
    };
    maOpacityMapModeMask.oninput = function() {
        maMaterialData.opacityMapMode = maOpacityMapModeMask.value;
        maGenerateJsonAndSave();         
    };
    maOpacityMapModeBlend.oninput = function() {
        maMaterialData.opacityMapMode = maOpacityMapModeBlend.value;
        maGenerateJsonAndSave(); 
    };    
    maOpacityCutoffSlider.oninput = function() {
        maOpacityCutoff.value = maOpacityCutoffSlider.value/1000;
        maMaterialData.opacityCutoff = parseFloat(maOpacityCutoff.value);
        maGenerateJsonAndSave();
    };
    maBloomSlider.oninput = function() {
        maBloom.value = maBloomSlider.value/100;
        maMaterialData.bloom = parseFloat(maBloom.value);
        maGenerateJsonAndSave();
    };
    maUnlit.oninput = function() {
        maMaterialData.unlit = maUnlit.checked;
        maGenerateJsonAndSave();
    };
    maEmissiveMap.oninput = function() {
        maMaterialData.emissiveMap = maEmissiveMap.value;
        maGenerateJsonAndSave(); 
    };
    maScatteringSlider.oninput = function() {
        maScattering.value = maScatteringSlider.value/1000;
        maMaterialData.scattering = parseFloat(maScattering.value);
        maGenerateJsonAndSave();
    };
    maScatteringMap.oninput = function() {
        maMaterialData.scatteringMap = maScatteringMap.value;
        maGenerateJsonAndSave(); 
    };
    maOcclusionMap.oninput = function() {
        maMaterialData.occlusionMap = maOcclusionMap.value;
        maGenerateJsonAndSave(); 
    };
    maLightMap.oninput = function() {
        maMaterialData.lightMap = maLightMap.value;
        maGenerateJsonAndSave(); 
    };
    maShadeIsActive.onclick = function() {
        if (maMaterialData.shadeIsActive) {
            maMaterialData.shadeIsActive = false;
            maShadeIsActive.className = "uiMaterialAssistant-inactive";
            maShadeColorPicker.style.pointerEvents = 'none';
            maShadeColorPicker.style.backgroundColor = "#555555";
        } else {
            maMaterialData.shadeIsActive = true;
            maShadeIsActive.className = "uiMaterialAssistant-active";
            maShadeColorPicker.style.pointerEvents = 'auto';
            maShadeColorPicker.style.backgroundColor = maGetRGB(maMaterialData.shade);
        }
        maGenerateJsonAndSave();
    };
    maShadeMapIsActive.onclick = function() {
        if (maMaterialData.shadeMapIsActive) {
            maMaterialData.shadeMapIsActive = false;
            maShadeMapIsActive.className = "uiMaterialAssistant-inactive";
            maShadeMap.disabled = true;
        } else {
            maMaterialData.shadeMapIsActive = true;
            maShadeMapIsActive.className = "uiMaterialAssistant-active";
            maShadeMap.disabled = false;
            if (maMaterialData.shadeMap === undefined) {
                maMaterialData.shadeMap = "";
            }
        }
        maGenerateJsonAndSave();
    };

    maShadeMap.oninput = function() {
        maMaterialData.shadeMap = maShadeMap.value;
        maGenerateJsonAndSave();
    };

    maMatcapIsActive.onclick = function() {
        if (maMaterialData.matcapIsActive) {
            maMaterialData.matcapIsActive = false;
            maMatcapIsActive.className = "uiMaterialAssistant-inactive";
            maMatcapColorPicker.style.pointerEvents = 'none';
            maMatcapColorPicker.style.backgroundColor = "#555555";
        } else {
            maMaterialData.matcapIsActive = true;
            maMatcapIsActive.className = "uiMaterialAssistant-active";
            maMatcapColorPicker.style.pointerEvents = 'auto';
            maMatcapColorPicker.style.backgroundColor = maGetRGB(maMaterialData.matcap);
        }
        maGenerateJsonAndSave();
    };
    maMatcapMapIsActive.onclick = function() {
        if (maMaterialData.matcapMapIsActive) {
            maMaterialData.matcapMapIsActive = false;
            maMatcapMapIsActive.className = "uiMaterialAssistant-inactive";
            maMatcapMap.disabled = true;
        } else {
            maMaterialData.matcapMapIsActive = true;
            maMatcapMapIsActive.className = "uiMaterialAssistant-active";
            maMatcapMap.disabled = false;
            if (maMaterialData.matcapMap === undefined) {
                maMaterialData.matcapMap = "";
            }
        }
        maGenerateJsonAndSave();
    };

    maMatcapMap.oninput = function() {
        maMaterialData.matcapMap = maMatcapMap.value;
        maGenerateJsonAndSave(); 
    };

    maParametricRimIsActive.onclick = function() {
        if (maMaterialData.parametricRimIsActive) {
            maMaterialData.parametricRimIsActive = false;
            maParametricRimIsActive.className = "uiMaterialAssistant-inactive";
            maParametricRimColorPicker.style.pointerEvents = 'none';
            maParametricRimColorPicker.style.backgroundColor = "#555555";
        } else {
            maMaterialData.parametricRimIsActive = true;
            maParametricRimIsActive.className = "uiMaterialAssistant-active";
            maParametricRimColorPicker.style.pointerEvents = 'auto';
            maParametricRimColorPicker.style.backgroundColor = maGetRGB(maMaterialData.parametricRim);
        }
        maGenerateJsonAndSave();
    };
    
    maRimMapIsActive.onclick = function() {
        if (maMaterialData.rimMapIsActive) {
            maMaterialData.rimMapIsActive = false;
            maRimMapIsActive.className = "uiMaterialAssistant-inactive";
            maRimMap.disabled = true;
        } else {
            maMaterialData.rimMapIsActive = true;
            maRimMapIsActive.className = "uiMaterialAssistant-active";
            maRimMap.disabled = false;
            if (maMaterialData.rimMap === undefined) {
                maMaterialData.rimMap = "";
            }
        }
        maGenerateJsonAndSave();
    };

    maRimMap.oninput = function() {
        maMaterialData.rimMap = maRimMap.value;
        maGenerateJsonAndSave(); 
    };

    maRimLightingMixIsActive.onclick = function() {
        if (maMaterialData.rimLightingMixIsActive) {
            maMaterialData.rimLightingMixIsActive = false;
            maRimLightingMixIsActive.className = "uiMaterialAssistant-inactive";
            maRimLightingMix.disabled = true;
            maRimLightingMixSlider.disabled = true;
        } else {
            maMaterialData.rimLightingMixIsActive = true;
            maRimLightingMixIsActive.className = "uiMaterialAssistant-active";
            maRimLightingMix.disabled = false;
            maRimLightingMixSlider.disabled = false;
        }
        maGenerateJsonAndSave();
    };
    
    maRimLightingMixSlider.oninput = function() {
        maRimLightingMix.value = maRimLightingMixSlider.value/1000;
        maMaterialData.rimLightingMix = parseFloat(maRimLightingMix.value);
        maGenerateJsonAndSave();
    };

    maParametricRimFresnelPowerIsActive.onclick = function() {
        if (maMaterialData.parametricRimFresnelPowerIsActive) {
            maMaterialData.parametricRimFresnelPowerIsActive = false;
            maParametricRimFresnelPowerIsActive.className = "uiMaterialAssistant-inactive";
            maParametricRimFresnelPower.disabled = true;
            maParametricRimFresnelPowerSlider.disabled = true;
        } else {
            maMaterialData.parametricRimFresnelPowerIsActive = true;
            maParametricRimFresnelPowerIsActive.className = "uiMaterialAssistant-active";
            maParametricRimFresnelPower.disabled = false;
            maParametricRimFresnelPowerSlider.disabled = false;
        }
        maGenerateJsonAndSave();
    };
    
    maParametricRimFresnelPowerSlider.oninput = function() {
        maParametricRimFresnelPower.value = maParametricRimFresnelPowerSlider.value/1000;
        maMaterialData.parametricRimFresnelPower = parseFloat(maParametricRimFresnelPower.value);
        maGenerateJsonAndSave();
    };

    maParametricRimLiftIsActive.onclick = function() {
        if (maMaterialData.parametricRimLiftIsActive) {
            maMaterialData.parametricRimLiftIsActive = false;
            maParametricRimLiftIsActive.className = "uiMaterialAssistant-inactive";
            maParametricRimLift.disabled = true;
            maParametricRimLiftSlider.disabled = true;
        } else {
            maMaterialData.parametricRimLiftIsActive = true;
            maParametricRimLiftIsActive.className = "uiMaterialAssistant-active";
            maParametricRimLift.disabled = false;
            maParametricRimLiftSlider.disabled = false;
        }
        maGenerateJsonAndSave();
    };
    
    maParametricRimLiftSlider.oninput = function() {
        maParametricRimLift.value = maParametricRimLiftSlider.value/1000;
        maMaterialData.parametricRimLift = parseFloat(maParametricRimLift.value);
        maGenerateJsonAndSave();
    };

    maShadingShiftIsActive.onclick = function() {
        if (maMaterialData.shadingShiftIsActive) {
            maMaterialData.shadingShiftIsActive = false;
            maShadingShiftIsActive.className = "uiMaterialAssistant-inactive";
            maShadingShift.disabled = true;
            maShadingShiftSlider.disabled = true;
        } else {
            maMaterialData.shadingShiftIsActive = true;
            maShadingShiftIsActive.className = "uiMaterialAssistant-active";
            maShadingShift.disabled = false;
            maShadingShiftSlider.disabled = false;
        }
        maGenerateJsonAndSave();
    };

    maShadingShiftMapIsActive.onclick = function() {
        if (maMaterialData.shadingShiftMapIsActive) {
            maMaterialData.shadingShiftMapIsActive = false;
            maShadingShiftMapIsActive.className = "uiMaterialAssistant-inactive";
            maShadingShiftMap.disabled = true;
        } else {
            maMaterialData.shadingShiftMapIsActive = true;
            maShadingShiftMapIsActive.className = "uiMaterialAssistant-active";
            maShadingShiftMap.disabled = false;
            if (maMaterialData.shadingShiftMap === undefined) {
                maMaterialData.shadingShiftMap = "";
            }
        }
        maGenerateJsonAndSave();
    };
    
    maShadingShiftSlider.oninput = function() {
        maShadingShift.value = maShadingShiftSlider.value/1000;
        maMaterialData.shadingShift = parseFloat(maShadingShift.value);
        maGenerateJsonAndSave();
    };
    
    maShadingShiftMap.oninput = function() {
        maMaterialData.shadingShiftMap = maShadingShiftMap.value;
        maGenerateJsonAndSave(); 
    };

    maShadingToonyIsActive.onclick = function() {
        if (maMaterialData.shadingToonyIsActive) {
            maMaterialData.shadingToonyIsActive = false;
            maShadingToonyIsActive.className = "uiMaterialAssistant-inactive";
            maShadingToony.disabled = true;
            maShadingToonySlider.disabled = true;
        } else {
            maMaterialData.shadingToonyIsActive = true;
            maShadingToonyIsActive.className = "uiMaterialAssistant-active";
            maShadingToony.disabled = false;
            maShadingToonySlider.disabled = false;
        }
        maGenerateJsonAndSave();
    };
    
    maShadingToonySlider.oninput = function() {
        maShadingToony.value = maShadingToonySlider.value/1000;
        maMaterialData.shadingToony = parseFloat(maShadingToony.value);
        maGenerateJsonAndSave();
    };

/* ################# Not supported Yet, but the code is ready #########################

    maOutlineWidthModeIsActive.onclick = function() {
        if (maMaterialData.outlineWidthModeIsActive) {
            maMaterialData.outlineWidthModeIsActive = false;
            maOutlineWidthModeIsActive.className = "uiMaterialAssistant-inactive";
            maOutlineWidthModeNone.disabled = true;
            maOutlineWidthModeWorld.disabled = true;
            maOutlineWidthModeScreen.disabled = true;
        } else {
            maMaterialData.outlineWidthModeIsActive = true;
            maOutlineWidthModeIsActive.className = "uiMaterialAssistant-active";
            maOutlineWidthModeNone.disabled = false;
            maOutlineWidthModeWorld.disabled = false;
            maOutlineWidthModeScreen.disabled = false;
        }
        maGenerateJsonAndSave(); 
    };
    
    maOutlineWidthModeNone.oninput = function() {
        maMaterialData.outlineWidthMode = maOutlineWidthModeNone.value;
        maGenerateJsonAndSave(); 
    };
    
    maOutlineWidthModeWorld.oninput = function() {
        maMaterialData.outlineWidthMode = maOutlineWidthModeWorld.value;
        maGenerateJsonAndSave(); 
    };
    
    maOutlineWidthModeScreen.oninput = function() {
        maMaterialData.outlineWidthMode = maOutlineWidthModeScreen.value;
        maGenerateJsonAndSave();
    };

    maOutlineWidthIsActive.onclick = function() {
        if (maMaterialData.outlineWidthIsActive) {
            maMaterialData.outlineWidthIsActive = false;
            maOutlineWidthIsActive.className = "uiMaterialAssistant-inactive";
            maOutlineWidth.disabled = true;
            maOutlineWidthSlider.disabled = true;
        } else {
            maMaterialData.outlineWidthIsActive = true;
            maOutlineWidthIsActive.className = "uiMaterialAssistant-active";
            maOutlineWidth.disabled = false;
            maOutlineWidthSlider.disabled = false;
        }
        maGenerateJsonAndSave();
    };
    
    maOutlineWidthSlider.oninput = function() {
        maOutlineWidth.value = maOutlineWidthSlider.value/1000;
        maMaterialData.outlineWidth = parseFloat(maOutlineWidth.value);
        maGenerateJsonAndSave();
    };

    maOutlineIsActive.onclick = function() {
        if (maMaterialData.outlineIsActive) {
            maMaterialData.outlineIsActive = false;
            maOutlineIsActive.className = "uiMaterialAssistant-inactive";
            maOutlineColorPicker.style.pointerEvents = 'none';
            maOutlineColorPicker.style.backgroundColor = "#555555";
        } else {
            maMaterialData.outlineIsActive = true;
            maOutlineIsActive.className = "uiMaterialAssistant-active";
            maOutlineColorPicker.style.pointerEvents = 'auto';
            maOutlineColorPicker.style.backgroundColor = maGetRGB(maMaterialData.outline);
        }
        maGenerateJsonAndSave();
    };
//################################################################
*/

    maUvAnimationMaskMapIsActive.onclick = function() {
        if (maMaterialData.uvAnimationMaskMapIsActive) {
            maMaterialData.uvAnimationMaskMapIsActive = false;
            maUvAnimationMaskMapIsActive.className = "uiMaterialAssistant-inactive";
            maUvAnimationMaskMap.disabled = true;
        } else {
            maMaterialData.uvAnimationMaskMapIsActive = true;
            maUvAnimationMaskMapIsActive.className = "uiMaterialAssistant-active";
            maUvAnimationMaskMap.disabled = false;
            if (maMaterialData.uvAnimationMaskMap === undefined) {
                maMaterialData.uvAnimationMaskMap = "";
            }
        }
        maGenerateJsonAndSave();
    };

    maUvAnimationMaskMap.oninput = function() {
        maMaterialData.uvAnimationMaskMap = maUvAnimationMaskMap.value;
        maGenerateJsonAndSave(); 
    };

    maUvAnimationScrollXSpeedIsActive.onclick = function() {
        if (maMaterialData.uvAnimationScrollXSpeedIsActive) {
            maMaterialData.uvAnimationScrollXSpeedIsActive = false;
            maUvAnimationScrollXSpeedIsActive.className = "uiMaterialAssistant-inactive";
            maUvAnimationScrollXSpeed.disabled = true;
            maUvAnimationScrollXSpeedSlider.disabled = true;
        } else {
            maMaterialData.uvAnimationScrollXSpeedIsActive = true;
            maUvAnimationScrollXSpeedIsActive.className = "uiMaterialAssistant-active";
            maUvAnimationScrollXSpeed.disabled = false;
            maUvAnimationScrollXSpeedSlider.disabled = false;
        }
        maGenerateJsonAndSave();
    };
    
    maUvAnimationScrollXSpeedSlider.oninput = function() {
        maUvAnimationScrollXSpeed.value = maUvAnimationScrollXSpeedSlider.value/1000;
        maMaterialData.uvAnimationScrollXSpeed = parseFloat(maUvAnimationScrollXSpeed.value);
        maGenerateJsonAndSave();
    };

    maUvAnimationScrollYSpeedIsActive.onclick = function() {
        if (maMaterialData.uvAnimationScrollYSpeedIsActive) {
            maMaterialData.uvAnimationScrollYSpeedIsActive = false;
            maUvAnimationScrollYSpeedIsActive.className = "uiMaterialAssistant-inactive";
            maUvAnimationScrollYSpeed.disabled = true;
            maUvAnimationScrollYSpeedSlider.disabled = true;
        } else {
            maMaterialData.uvAnimationScrollYSpeedIsActive = true;
            maUvAnimationScrollYSpeedIsActive.className = "uiMaterialAssistant-active";
            maUvAnimationScrollYSpeed.disabled = false;
            maUvAnimationScrollYSpeedSlider.disabled = false;
        }
        maGenerateJsonAndSave();
    };
    
    maUvAnimationScrollYSpeedSlider.oninput = function() {
        maUvAnimationScrollYSpeed.value = maUvAnimationScrollYSpeedSlider.value/1000;
        maMaterialData.uvAnimationScrollYSpeed = parseFloat(maUvAnimationScrollYSpeed.value);
        maGenerateJsonAndSave();
    };

    maUvAnimationRotationSpeedIsActive.onclick = function() {
        if (maMaterialData.uvAnimationRotationSpeedIsActive) {
            maMaterialData.uvAnimationRotationSpeedIsActive = false;
            maUvAnimationRotationSpeedIsActive.className = "uiMaterialAssistant-inactive";
            maUvAnimationRotationSpeed.disabled = true;
            maUvAnimationRotationSpeedSlider.disabled = true;
        } else {
            maMaterialData.uvAnimationRotationSpeedIsActive = true;
            maUvAnimationRotationSpeedIsActive.className = "uiMaterialAssistant-active";
            maUvAnimationRotationSpeed.disabled = false;
            maUvAnimationRotationSpeedSlider.disabled = false;
        }
        maGenerateJsonAndSave();
    };
    
    maUvAnimationRotationSpeedSlider.oninput = function() {
        maUvAnimationRotationSpeed.value = maUvAnimationRotationSpeedSlider.value/1000;
        maMaterialData.uvAnimationRotationSpeed = parseFloat(maUvAnimationRotationSpeed.value);
        maGenerateJsonAndSave();
    };

    maProcedural.oninput = function() {
        let testJsonData;
        if (maProcedural.value === "") {
            maAddProceduralTemplate.disabled = false;
        } else {
            try {
                testJsonData = JSON.parse(maProcedural.value);
            } catch (e) {
                maProcedural.style.color = "#ff0000";
                return;
            }
        }
        maProcedural.style.color = "#000000";
        maMaterialData.procedural = maProcedural.value;
        maGenerateJsonAndSave();
    };
    maAddProceduralTemplate.onclick = function() {
        if (maMaterialData.procedural === "") {
            let template = {
                "version": 3,
                "fragmentShaderURL": "<your .fs file url>",
                "uniforms": {},
                "channels": []
            };
            maProcedural.value = JSON.stringify(template, null, 4);
            maAddProceduralTemplate.disabled = true;
            maProcedural.oninput();
        }
    };
    maCullFaceModeBack.oninput = function() {
        maMaterialData.cullFaceMode = maCullFaceModeBack.value;
        maGenerateJsonAndSave(); 
    };
    maCullFaceModeFront.oninput = function() {
        maMaterialData.cullFaceMode = maCullFaceModeFront.value;
        maGenerateJsonAndSave(); 
    };
    maCullFaceModeNone.oninput = function() {
        maMaterialData.cullFaceMode = maCullFaceModeNone.value;
        maGenerateJsonAndSave();
    };
    
    maModelHifiPbr.oninput = function() {
        maMaterialData.model = maModelHifiPbr.value;
        maFieldContextualDisplayer();
        maGenerateJsonAndSave();
    };
    maModelVrmMtoon.oninput = function() {
        maMaterialData.model = maModelVrmMtoon.value;
        maFieldContextualDisplayer();
        maGenerateJsonAndSave();
    };
    maModelHifiShaderSimple.oninput = function() {
        maMaterialData.model = maModelHifiShaderSimple.value;
        maFieldContextualDisplayer();
        maGenerateJsonAndSave();
    };
    
    var maAlbedoColorPickerID = "#ma-albedo-colorPicker";
    maAlbedoColorPickerSel = $(maAlbedoColorPickerID).colpick({
        colorScheme: 'dark',
        layout: 'rgbhex',
        color: '000000',
        submit: false,
        onShow: function(colpick) {
            $(maAlbedoColorPickerID).colpickSetColor({
                "r": maMaterialData.albedo[0] * 255,
                "g": maMaterialData.albedo[1] * 255,
                "b": maMaterialData.albedo[2] * 255,
            });
            $(maAlbedoColorPickerID).attr('active', 'true');
        },
        onHide: function(colpick) {
            $(maAlbedoColorPickerID).attr('active', 'false');
        },
        onChange: function(hsb, hex, rgb, el) {
            $(el).css('background-color', '#' + hex);
            if ($(maAlbedoColorPickerID).attr('active') === 'true') {
                maMaterialData.albedo = [(rgb.r/255), (rgb.g/255), (rgb.b/255)];
                maGenerateJsonAndSave();
            }
        }
    });
    
    var maEmissiveColorPickerID = "#ma-emissive-colorPicker";
    maEmissiveColorPickerSel = $(maEmissiveColorPickerID).colpick({
        colorScheme: 'dark',
        layout: 'rgbhex',
        color: '000000',
        submit: false,
        onShow: function(colpick) {
            $(maEmissiveColorPickerID).colpickSetColor({
                "r": maMaterialData.emissive[0] * 255,
                "g": maMaterialData.emissive[1] * 255,
                "b": maMaterialData.emissive[2] * 255,
            });
            $(maEmissiveColorPickerID).attr('active', 'true');
        },
        onHide: function(colpick) {
            $(maEmissiveColorPickerID).attr('active', 'false');
        },
        onChange: function(hsb, hex, rgb, el) {
            $(el).css('background-color', '#' + hex);
            if ($(maEmissiveColorPickerID).attr('active') === 'true') {
                maMaterialData.emissive = [(rgb.r/255), (rgb.g/255), (rgb.b/255)];
                maGenerateJsonAndSave();
            }
        }
    });
    
    var maShadeColorPickerID = "#ma-shade-colorPicker";
    maShadeColorPickerSel = $(maShadeColorPickerID).colpick({
        colorScheme: 'dark',
        layout: 'rgbhex',
        color: '000000',
        submit: false,
        onShow: function(colpick) {
            $(maShadeColorPickerID).colpickSetColor({
                "r": maMaterialData.shade[0] * 255,
                "g": maMaterialData.shade[1] * 255,
                "b": maMaterialData.shade[2] * 255,
            });
            $(maShadeColorPickerID).attr('active', 'true');
        },
        onHide: function(colpick) {
            $(maShadeColorPickerID).attr('active', 'false');
        },
        onChange: function(hsb, hex, rgb, el) {
            $(el).css('background-color', '#' + hex);
            if ($(maShadeColorPickerID).attr('active') === 'true') {
                maMaterialData.shade = [(rgb.r/255), (rgb.g/255), (rgb.b/255)];
                maGenerateJsonAndSave();
            }
        }
    });
    
    var maMatcapColorPickerID = "#ma-matcap-colorPicker";
    maMatcapColorPickerSel = $(maMatcapColorPickerID).colpick({
        colorScheme: 'dark',
        layout: 'rgbhex',
        color: '000000',
        submit: false,
        onShow: function(colpick) {
            $(maMatcapColorPickerID).colpickSetColor({
                "r": maMaterialData.matcap[0] * 255,
                "g": maMaterialData.matcap[1] * 255,
                "b": maMaterialData.matcap[2] * 255,
            });
            $(maMatcapColorPickerID).attr('active', 'true');
        },
        onHide: function(colpick) {
            $(maMatcapColorPickerID).attr('active', 'false');
        },
        onChange: function(hsb, hex, rgb, el) {
            $(el).css('background-color', '#' + hex);
            if ($(maMatcapColorPickerID).attr('active') === 'true') {
                maMaterialData.matcap = [(rgb.r/255), (rgb.g/255), (rgb.b/255)];
                maGenerateJsonAndSave();
            }
        }
    });
    
    var maParametricRimColorPickerID = "#ma-parametricRim-colorPicker";
    maParametricRimColorPickerSel = $(maParametricRimColorPickerID).colpick({
        colorScheme: 'dark',
        layout: 'rgbhex',
        color: '000000',
        submit: false,
        onShow: function(colpick) {
            $(maParametricRimColorPickerID).colpickSetColor({
                "r": maMaterialData.parametricRim[0] * 255,
                "g": maMaterialData.parametricRim[1] * 255,
                "b": maMaterialData.parametricRim[2] * 255,
            });
            $(maParametricRimColorPickerID).attr('active', 'true');
        },
        onHide: function(colpick) {
            $(maParametricRimColorPickerID).attr('active', 'false');
        },
        onChange: function(hsb, hex, rgb, el) {
            $(el).css('background-color', '#' + hex);
            if ($(maParametricRimColorPickerID).attr('active') === 'true') {
                maMaterialData.parametricRim = [(rgb.r/255), (rgb.g/255), (rgb.b/255)];
                maGenerateJsonAndSave();
            }
        }
    });

/* ############# Not supported yet, but the code is ready #########################

    var maOutlineColorPickerID = "#ma-outline-colorPicker";
    maOutlineColorPickerSel = $(maOutlineColorPickerID).colpick({
        colorScheme: 'dark',
        layout: 'rgbhex',
        color: '000000',
        submit: false,
        onShow: function(colpick) {
            $(maOutlineColorPickerID).colpickSetColor({
                "r": maMaterialData.outline[0] * 255,
                "g": maMaterialData.outline[1] * 255,
                "b": maMaterialData.outline[2] * 255,
            });
            $(maOutlineColorPickerID).attr('active', 'true');
        },
        onHide: function(colpick) {
            $(maOutlineColorPickerID).attr('active', 'false');
        },
        onChange: function(hsb, hex, rgb, el) {
            $(el).css('background-color', '#' + hex);
            if ($(maOutlineColorPickerID).attr('active') === 'true') {
                maMaterialData.outline = [(rgb.r/255), (rgb.g/255), (rgb.b/255)];
                maGenerateJsonAndSave();
            }
        }
    });
//##############################################################################
*/

}

function loadDataInMaUi(materialDataPropertyValue) {
    var entityMaterialData, materialDefinition;
    var mapToUse;

    if (materialDataPropertyValue === "" || materialDataPropertyValue === "{}") {
        entityMaterialData = {
                "materials":{}
            };
    } else {
        try {
            entityMaterialData = JSON.parse(materialDataPropertyValue);
        }
        catch(e) {
            entityMaterialData = {
                    "materials":{}
                };
        }
    }
    
    if (entityMaterialData.materials.length === undefined) {
        materialDefinition = entityMaterialData.materials;
    } else {
        materialDefinition = entityMaterialData.materials[0];
    }

    //MODEL
    maMaterialData.model = materialDefinition.model || DEFAULT_MODEL;
    switch (maMaterialData.model) {
        case "hifi_pbr":
            maModelHifiPbr.checked = true;
            break;
        case "vrm_mtoon":
            maModelVrmMtoon.checked = true;
            break;
        case "hifi_shader_simple":
            maModelHifiShaderSimple.checked = true;
            break;
        default:
            alert("ERROR: Unrecognized material model. (model = '" + maMaterialData.model + "')");
    }

    //NAME
    if (materialDefinition.name !== undefined) {
        maMaterialData.name = materialDefinition.name;
    } else {
        maMaterialData.name = "";
    }
    maName.value = maMaterialData.name;
    
    //ALBEDO
    if (materialDefinition.defaultFallthrough === true && materialDefinition.albedo === undefined) {
        maMaterialData.albedoIsActive = false;
        maAlbedoIsActive.className = "uiMaterialAssistant-inactive";
        maAlbedoColorPicker.style.pointerEvents = 'none';
        maAlbedoColorPicker.style.backgroundColor = "#555555";
    } else {
        maMaterialData.albedoIsActive = true;
        maAlbedoIsActive.className = "uiMaterialAssistant-active";
        maAlbedoColorPicker.style.pointerEvents = 'auto';
        maAlbedoColorPicker.style.backgroundColor = maGetRGB(maMaterialData.albedo);
    }        
    
    if (materialDefinition.albedo !== undefined) {
        maMaterialData.albedo = materialDefinition.albedo;
    } else {
        maMaterialData.albedo = DEFAULT_ALBEDO;
    }
    maAlbedoColorPicker.style.backgroundColor = maGetRGB(maMaterialData.albedo);

    //ALBEDO MAP
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.albedoMap === undefined 
            && (materialDefinition.model === "hifi_pbr" || materialDefinition.model === "vrm_mtoon" )) {
        maMaterialData.albedoMapIsActive = false;
        maAlbedoMapIsActive.className = "uiMaterialAssistant-inactive";
        maAlbedoMap.disabled = true;
    } else {
        maMaterialData.albedoMapIsActive = true;
        maAlbedoMapIsActive.className = "uiMaterialAssistant-active";
        maAlbedoMap.disabled = false;
    } 

    if (materialDefinition.albedoMap !== undefined) {
        maMaterialData.albedoMap = materialDefinition.albedoMap;
    } else {
        maMaterialData.albedoMap = "";
    }
    maAlbedoMap.value = maMaterialData.albedoMap;
    
    //METALLIC
    if (materialDefinition.metallicMap !== undefined && materialDefinition.specularMap !== undefined) {
        if (JSON.stringify(materialDefinition).indexOf("metallicMap") > JSON.stringify(materialDefinition).indexOf("specularMap")) {
            delete materialDefinition.specularMap;
            mapToUse = materialDefinition.metallicMap;
            maUseSpecular.checked = false;
        } else {
            delete materialDefinition.metallicMap;
            mapToUse = materialDefinition.specularMap;
            maUseSpecular.checked = true;
        }
    } else if (materialDefinition.metallicMap === undefined && materialDefinition.specularMap === undefined) {
        mapToUse = materialDefinition.metallicMap;
        maUseSpecular.checked = false;
    } else if (materialDefinition.specularMap === undefined) {
        mapToUse = materialDefinition.metallicMap;
        maUseSpecular.checked = false;
    } else {
        mapToUse = materialDefinition.specularMap;
        maUseSpecular.checked = true;
    }
    
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.metallic === undefined 
            && mapToUse === undefined
            && materialDefinition.model === "hifi_pbr") {
        maMaterialData.metallicIsActive = false;
        maMetallicIsActive.className = "uiMaterialAssistant-inactive";
        maMetallic.disabled = true;
        maMetallicSlider.disabled = true;
        maMetallicMap.disabled = true;
        maUseSpecular.disabled = true;
    } else {
        maMaterialData.metallicIsActive = true;
        maMetallicIsActive.className = "uiMaterialAssistant-active";
        maMetallic.disabled = false;
        maMetallicSlider.disabled = false;
        maMetallicMap.disabled = false;
        maUseSpecular.disabled = false;
    } 

    if (materialDefinition.metallic !== undefined) {
        maMaterialData.metallic = materialDefinition.metallic;
    } else {
        maMaterialData.metallic = DEFAULT_METALLIC_FOR_MA_UI;
    }
    maMetallic.value = maMaterialData.metallic;
    maMetallicSlider.value = Math.floor(maMaterialData.metallic * 1000);
    
    if (mapToUse !== undefined) {
        maMaterialData.metallicMap = mapToUse;
    } else {
        maMaterialData.metallicMap = "";
    }
    maMetallicMap.value = maMaterialData.metallicMap;
    maMaterialData.useSpecular = maUseSpecular.checked;

    //ROUGHNESS
    if (materialDefinition.roughnessMap !== undefined && materialDefinition.glossMap !== undefined) {
        if (JSON.stringify(materialDefinition).indexOf("roughnessMap") > JSON.stringify(materialDefinition).indexOf("glossMap")) {
            delete materialDefinition.glossMap;
            mapToUse = materialDefinition.roughnessMap;
            maUseGloss.checked = false;
        } else {
            delete materialDefinition.roughnessMap;
            mapToUse = materialDefinition.glossMap;
            maUseGloss.checked = true;
        }
    } else if (materialDefinition.roughnessMap === undefined && materialDefinition.glossMap === undefined) {
        mapToUse = materialDefinition.roughnessMap;
        maUseGloss.checked = false;
    } else if (materialDefinition.glossMap === undefined) {
        mapToUse = materialDefinition.roughnessMap;
        maUseGloss.checked = false;
    } else {
        mapToUse = materialDefinition.glossMap;
        maUseGloss.checked = true;
    }
    
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.roughness === undefined 
            && mapToUse === undefined
            && materialDefinition.model === "hifi_pbr") {
        maMaterialData.roughnessIsActive = false;
        maRoughnessIsActive.className = "uiMaterialAssistant-inactive";
        maRoughness.disabled = true;
        maRoughnessSlider.disabled = true;
        maRoughnessMap.disabled = true;
        maUseGloss.disabled = true;
    } else {
        maMaterialData.roughnessIsActive = true;
        maRoughnessIsActive.className = "uiMaterialAssistant-active";
        maRoughness.disabled = false;
        maRoughnessSlider.disabled = false;
        maRoughnessMap.disabled = false;
        maUseGloss.disabled = false;
    } 

    if (materialDefinition.roughness !== undefined) {
        maMaterialData.roughness = materialDefinition.roughness;
    } else {
        maMaterialData.roughness = DEFAULT_ROUGHNESS;
    }
    maRoughness.value = maMaterialData.roughness;
    maRoughnessSlider.value = Math.floor(maMaterialData.roughness * 1000);
    
    if (mapToUse !== undefined) {
        maMaterialData.roughnessMap = mapToUse;
    } else {
        maMaterialData.roughnessMap = "";
    }
    maRoughnessMap.value = maMaterialData.roughnessMap;
    maMaterialData.useGloss = maUseGloss.checked;

    //NORMAL MAP
    if (materialDefinition.normalMap !== undefined && materialDefinition.bumpMap !== undefined) {
        if (JSON.stringify(materialDefinition).indexOf("normalMap") > JSON.stringify(materialDefinition).indexOf("bumpMap")) {
            delete materialDefinition.bumpMap;
            mapToUse = materialDefinition.normalMap;
            maUseBump.checked = false;
        } else {
            delete materialDefinition.normalMap;
            mapToUse = materialDefinition.bumpMap;
            maUseBump.checked = true;
        }
    } else if (materialDefinition.normalMap === undefined && materialDefinition.bumpMap === undefined) {
        mapToUse = materialDefinition.normalMap;
        maUseBump.checked = false;
    } else if (materialDefinition.bumpMap === undefined) {
        mapToUse = materialDefinition.normalMap;
        maUseBump.checked = false;
    } else {
        mapToUse = materialDefinition.bumpMap;
        maUseBump.checked = true;
    }
    
    if (materialDefinition.defaultFallthrough === true && 
            mapToUse === undefined &&
            (materialDefinition.model === "hifi_pbr" || materialDefinition.model === "vrm_mtoon")) {
        maMaterialData.normalMapIsActive = false;
        maNormalMapIsActive.className = "uiMaterialAssistant-inactive";
        maNormalMap.disabled = true;
        maUseBump.disabled = true;
    } else {
        maMaterialData.normalMapIsActive = true;
        maNormalMapIsActive.className = "uiMaterialAssistant-active";
        maNormalMap.disabled = false;
        maUseBump.disabled = false;
    }

    if (mapToUse !== undefined) {
        maMaterialData.normalMap = mapToUse;
    } else {
        maMaterialData.normalMap = "";
    }
    maNormalMap.value = maMaterialData.normalMap;
    maMaterialData.useBump = maUseBump.checked;

    //OPACITY
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.opacity === undefined 
            && materialDefinition.opacityMapMode === undefined) {
        maMaterialData.opacityIsActive = false;
        maOpacityIsActive.className = "uiMaterialAssistant-inactive";
        maOpacity.disabled = true;
        maOpacitySlider.disabled = true;
        maOpacityMapModeDont.disabled = true;
        maOpacityMapModeMask.disabled = true;
        maOpacityMapModeBlend.disabled = true;
        maOpacityCutoff.disabled = true;
        maOpacityCutoffSlider.disabled = true;
    } else {
        maMaterialData.opacityIsActive = true;
        maOpacityIsActive.className = "uiMaterialAssistant-active";
        maOpacity.disabled = false;
        maOpacitySlider.disabled = false;
        maOpacityMapModeDont.disabled = false;
        maOpacityMapModeMask.disabled = false;
        maOpacityMapModeBlend.disabled = false;
        maOpacityCutoff.disabled = false;
        maOpacityCutoffSlider.disabled = false;
    }
    
    if (materialDefinition.opacity !== undefined) {
        maMaterialData.opacity = materialDefinition.opacity;
    } else {
        maMaterialData.opacity = DEFAULT_OPACITY;
    }
    maOpacity.value = maMaterialData.opacity;
    maOpacitySlider.value = Math.floor(maMaterialData.opacity * 1000);

    if (materialDefinition.opacityMapMode !== undefined) {
        maMaterialData.opacityMapMode = materialDefinition.opacityMapMode;
    } else {
        maMaterialData.opacityMapMode = "OPACITY_MAP_OPAQUE";
    }
    switch (maMaterialData.opacityMapMode) {
        case "OPACITY_MAP_OPAQUE":
            maOpacityMapModeDont.checked = true;
            break;
        case "OPACITY_MAP_MASK":
            maOpacityMapModeMask.checked = true;
            break;
        case "OPACITY_MAP_BLEND":
            maOpacityMapModeBlend.checked = true;
            break;
        default:
            alert("ERROR: Unrecognized material opacityMapMode. (opacityMapMode = '" + maMaterialData.opacityMapMode + "')");
    }

    if (materialDefinition.opacityCutoff !== undefined) {
        maMaterialData.opacityCutoff = materialDefinition.opacityCutoff;
    } else {
        maMaterialData.opacityCutoff = DEFAULT_OPACITY_CUTOFF;
    }
    maOpacityCutoff.value = maMaterialData.opacityCutoff;
    maOpacityCutoffSlider.value = Math.floor(maMaterialData.opacityCutoff * 1000);

    //EMISSIVE
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.emissive === undefined 
            && materialDefinition.emissiveMap === undefined
            && (materialDefinition.model === "hifi_pbr" || materialDefinition.model === "vrm_mtoon")) {
        maMaterialData.emissiveIsActive = false;
        maEmissiveIsActive.className = "uiMaterialAssistant-inactive";
        maBloom.disabled = true;
        maBloomSlider.disabled = true;
        maEmissiveMap.disabled = true;
        maEmissiveColorPicker.style.pointerEvents = 'none';
        maEmissiveColorPicker.style.backgroundColor = "#555555";
    } else {
        maMaterialData.emissiveIsActive = true;
        maEmissiveIsActive.className = "uiMaterialAssistant-active";
        maBloom.disabled = false;
        maBloomSlider.disabled = false;
        maEmissiveMap.disabled = false;
        maEmissiveColorPicker.style.pointerEvents = 'auto';
        maEmissiveColorPicker.style.backgroundColor = maGetRGB(maMaterialData.emissive);
    }
    
    if (materialDefinition.emissive !== undefined) {
        maMaterialData.emissive = maGetColorValueFromEmissive(materialDefinition.emissive);
        maMaterialData.bloom = maGetBloomFactorFromEmissive(materialDefinition.emissive);
    } else {
        maMaterialData.emissive = DEFAULT_EMISSIVE;
        maMaterialData.bloom = DEFAULT_BLOOM_FACTOR;
    }
    maEmissiveColorPicker.style.backgroundColor = maGetRGB(maMaterialData.emissive);
    maBloom.value = maMaterialData.bloom;
    maBloomSlider.value = Math.floor(maMaterialData.bloom * 100);
    
    if (materialDefinition.emissiveMap !== undefined) {
        maMaterialData.emissiveMap = materialDefinition.emissiveMap;
    } else {
        maMaterialData.emissiveMap = "";
    }
    maEmissiveMap.value = maMaterialData.emissiveMap;
    
    //UNLIT
    if (materialDefinition.unlit !== undefined) {
        maMaterialData.unlit = materialDefinition.unlit;
    } else {
        maMaterialData.unlit = false;
    }
    maUnlit.checked = maMaterialData.unlit;
    
    //SCATTERING
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.scattering === undefined 
            && materialDefinition.scatteringMap === undefined
            && materialDefinition.model === "hifi_pbr") {
        maMaterialData.scatteringIsActive = false;
        maScatteringIsActive.className = "uiMaterialAssistant-inactive";
        maScattering.disabled = true;
        maScatteringSlider.disabled = true;
        maScatteringMap.disabled = true;
    } else {
        maMaterialData.scatteringIsActive = true;
        maScatteringIsActive.className = "uiMaterialAssistant-active";
        maScattering.disabled = false;
        maScatteringSlider.disabled = false;
        maScatteringMap.disabled = false;
    } 

    if (materialDefinition.scattering !== undefined) {
        maMaterialData.scattering = materialDefinition.scattering;
    } else {
        maMaterialData.scattering = DEFAULT_SCATTERING;
    }
    maScattering.value = maMaterialData.scattering;
    maScatteringSlider.value = Math.floor(maMaterialData.scattering * 1000);
    
    if (materialDefinition.scatteringMap !== undefined) {
        maMaterialData.scatteringMap = materialDefinition.scatteringMap;
    } else {
        maMaterialData.scatteringMap = "";
    }
    maScatteringMap.value = maMaterialData.scatteringMap;

    //OCCLUSION MAP
    if (materialDefinition.defaultFallthrough === true && materialDefinition.occlusionMap === undefined
            && materialDefinition.model === "hifi_pbr") {
        maMaterialData.occlusionMapIsActive = false;
        maOcclusionMapIsActive.className = "uiMaterialAssistant-inactive";
        maOcclusionMap.disabled = true;
    } else {
        maMaterialData.occlusionMapIsActive = true;
        maOcclusionMapIsActive.className = "uiMaterialAssistant-active";
        maOcclusionMap.disabled = false;
    }

    if (materialDefinition.occlusionMap !== undefined) {
        maMaterialData.occlusionMap = materialDefinition.occlusionMap;
    } else {
        maMaterialData.occlusionMap = "";
    }
    maOcclusionMap.value = maMaterialData.occlusionMap;

    //LIGHT MAP
    if (materialDefinition.defaultFallthrough === true && 
            materialDefinition.lightMap === undefined && 
            materialDefinition.model === "hifi_pbr") {
        maMaterialData.lightMapIsActive = false;
        maLightMapIsActive.className = "uiMaterialAssistant-inactive";
        maLightMap.disabled = true;
    } else {
        maMaterialData.lightMapIsActive = true;
        maLightMapIsActive.className = "uiMaterialAssistant-active";
        maLightMap.disabled = false;
    }

    if (materialDefinition.lightMap !== undefined) {
        maMaterialData.lightMap = materialDefinition.lightMap;
    } else {
        maMaterialData.lightMap = "";
    }
    maLightMap.value = maMaterialData.lightMap;

    //SHADE
    if (materialDefinition.defaultFallthrough === true && materialDefinition.shade === undefined
            && materialDefinition.model === "vrm_mtoon" ) {
        maMaterialData.shadeIsActive = false;
        maShadeIsActive.className = "uiMaterialAssistant-inactive";
        maShadeColorPicker.style.pointerEvents = 'none';
        maShadeColorPicker.style.backgroundColor = "#555555";
    } else {
        maMaterialData.shadeIsActive = true;
        maShadeIsActive.className = "uiMaterialAssistant-active";
        maShadeColorPicker.style.pointerEvents = 'auto';
        maShadeColorPicker.style.backgroundColor = maGetRGB(maMaterialData.shade);
    }        
    
    if (materialDefinition.shade !== undefined) {
        maMaterialData.shade = materialDefinition.shade;
    } else {
        maMaterialData.shade = DEFAULT_SHADE;
    }
    maShadeColorPicker.style.backgroundColor = maGetRGB(maMaterialData.shade);

    //SHADE MAP
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.shadeMap === undefined 
            && materialDefinition.model === "vrm_mtoon" ) {
        maMaterialData.shadeMapIsActive = false;
        maShadeMapIsActive.className = "uiMaterialAssistant-inactive";
        maShadeMap.disabled = true;
    } else {
        maMaterialData.shadeMapIsActive = true;
        maShadeMapIsActive.className = "uiMaterialAssistant-active";
        maShadeMap.disabled = false;
    } 

    if (materialDefinition.shadeMap !== undefined) {
        maMaterialData.shadeMap = materialDefinition.shadeMap;
    } else {
        maMaterialData.shadeMap = "";
    }
    maShadeMap.value = maMaterialData.shadeMap;

    //MATCAP
    if (materialDefinition.defaultFallthrough === true && materialDefinition.matcap === undefined
            && materialDefinition.model === "vrm_mtoon" ) {
        maMaterialData.matcapIsActive = false;
        maMatcapIsActive.className = "uiMaterialAssistant-inactive";
        maMatcapColorPicker.style.pointerEvents = 'none';
        maMatcapColorPicker.style.backgroundColor = "#555555";
    } else {
        maMaterialData.matcapIsActive = true;
        maMatcapIsActive.className = "uiMaterialAssistant-active";
        maMatcapColorPicker.style.pointerEvents = 'auto';
        maMatcapColorPicker.style.backgroundColor = maGetRGB(maMaterialData.matcap);
    }        
    
    if (materialDefinition.matcap !== undefined) {
        maMaterialData.matcap = materialDefinition.matcap;
    } else {
        maMaterialData.matcap = DEFAULT_MATCAP;
    }
    maMatcapColorPicker.style.backgroundColor = maGetRGB(maMaterialData.matcap);

    //MATCAP MAP
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.matcapMap === undefined 
            && materialDefinition.model === "vrm_mtoon" ) {
        maMaterialData.matcapMapIsActive = false;
        maMatcapMapIsActive.className = "uiMaterialAssistant-inactive";
        maMatcapMap.disabled = true;
    } else {
        maMaterialData.matcapMapIsActive = true;
        maMatcapMapIsActive.className = "uiMaterialAssistant-active";
        maMatcapMap.disabled = false;
    } 

    if (materialDefinition.matcapMap !== undefined) {
        maMaterialData.matcapMap = materialDefinition.matcapMap;
    } else {
        maMaterialData.matcapMap = "";
    }
    maMatcapMap.value = maMaterialData.matcapMap;

    //SHADING SHIFT
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.shadingShift === undefined 
            && materialDefinition.model === "vrm_mtoon") {
        maMaterialData.shadingShiftIsActive = false;
        maShadingShiftIsActive.className = "uiMaterialAssistant-inactive";
        maShadingShift.disabled = true;
        maShadingShiftSlider.disabled = true;
    } else {
        maMaterialData.shadingShiftIsActive = true;
        maShadingShiftIsActive.className = "uiMaterialAssistant-active";
        maShadingShift.disabled = false;
        maShadingShiftSlider.disabled = false;
    } 

    if (materialDefinition.shadingShift !== undefined) {
        maMaterialData.shadingShift = materialDefinition.shadingShift;
    } else {
        maMaterialData.shadingShift = DEFAULT_SHADING_SHIFT;
    }
    maShadingShift.value = maMaterialData.shadingShift;
    maShadingShiftSlider.value = Math.floor(maMaterialData.shadingShift * 1000);
    
    //SHADING SHIFT MAP
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.shadingShiftMap === undefined 
            && materialDefinition.model === "vrm_mtoon" ) {
        maMaterialData.shadingShiftMapIsActive = false;
        maShadingShiftMapIsActive.className = "uiMaterialAssistant-inactive";
        maShadingShiftMap.disabled = true;
    } else {
        maMaterialData.shadingShiftMapIsActive = true;
        maShadingShiftMapIsActive.className = "uiMaterialAssistant-active";
        maShadingShiftMap.disabled = false;
    } 

    if (materialDefinition.shadingShiftMap !== undefined) {
        maMaterialData.shadingShiftMap = materialDefinition.shadingShiftMap;
    } else {
        maMaterialData.shadingShiftMap = "";
    }
    maShadingShiftMap.value = maMaterialData.shadingShiftMap;

    //SHADING TOONY
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.shadingToony === undefined 
            && materialDefinition.model === "vrm_mtoon") {
        maMaterialData.shadingToonyIsActive = false;
        maShadingToonyIsActive.className = "uiMaterialAssistant-inactive";
        maShadingToony.disabled = true;
        maShadingToonySlider.disabled = true;
    } else {
        maMaterialData.shadingToonyIsActive = true;
        maShadingToonyIsActive.className = "uiMaterialAssistant-active";
        maShadingToony.disabled = false;
        maShadingToonySlider.disabled = false;
    } 

    if (materialDefinition.shadingToony !== undefined) {
        maMaterialData.shadingToony = materialDefinition.shadingToony;
    } else {
        maMaterialData.shadingToony = DEFAULT_SHADING_TOONY;
    }
    maShadingToony.value = maMaterialData.shadingToony;
    maShadingToonySlider.value = Math.floor(maMaterialData.shadingToony * 1000);

    //PARAMETRIC RIM
    if (materialDefinition.defaultFallthrough === true && materialDefinition.parametricRim === undefined
            && materialDefinition.model === "vrm_mtoon" ) {
        maMaterialData.parametricRimIsActive = false;
        maParametricRimIsActive.className = "uiMaterialAssistant-inactive";
        maParametricRimColorPicker.style.pointerEvents = 'none';
        maParametricRimColorPicker.style.backgroundColor = "#555555";
    } else {
        maMaterialData.parametricRimIsActive = true;
        maParametricRimIsActive.className = "uiMaterialAssistant-active";
        maParametricRimColorPicker.style.pointerEvents = 'auto';
        maParametricRimColorPicker.style.backgroundColor = maGetRGB(maMaterialData.parametricRim);
    }        
    
    if (materialDefinition.parametricRim !== undefined) {
        maMaterialData.parametricRim = materialDefinition.parametricRim;
    } else {
        maMaterialData.parametricRim = DEFAULT_PARAMETRIC_RIM;
    }
    maParametricRimColorPicker.style.backgroundColor = maGetRGB(maMaterialData.parametricRim);

    //RIM MAP
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.rimMap === undefined 
            && materialDefinition.model === "vrm_mtoon" ) {
        maMaterialData.rimMapIsActive = false;
        maRimMapIsActive.className = "uiMaterialAssistant-inactive";
        maRimMap.disabled = true;
    } else {
        maMaterialData.rimMapIsActive = true;
        maRimMapIsActive.className = "uiMaterialAssistant-active";
        maRimMap.disabled = false;
    } 

    if (materialDefinition.rimMap !== undefined) {
        maMaterialData.rimMap = materialDefinition.rimMap;
    } else {
        maMaterialData.rimMap = "";
    }
    maRimMap.value = maMaterialData.rimMap;

    //RIM LIGHTING MIX
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.rimLightingMix === undefined 
            && materialDefinition.model === "vrm_mtoon") {
        maMaterialData.rimLightingMixIsActive = false;
        maRimLightingMixIsActive.className = "uiMaterialAssistant-inactive";
        maRimLightingMix.disabled = true;
        maRimLightingMixSlider.disabled = true;
    } else {
        maMaterialData.rimLightingMixIsActive = true;
        maRimLightingMixIsActive.className = "uiMaterialAssistant-active";
        maRimLightingMix.disabled = false;
        maRimLightingMixSlider.disabled = false;
    } 

    if (materialDefinition.rimLightingMix !== undefined) {
        maMaterialData.rimLightingMix = materialDefinition.rimLightingMix;
    } else {
        maMaterialData.rimLightingMix = DEFAULT_RIM_LIGHTING_MIX;
    }
    maRimLightingMix.value = maMaterialData.rimLightingMix;
    maRimLightingMixSlider.value = Math.floor(maMaterialData.rimLightingMix * 1000);

    //PARAMETRIC RIM FRESNEL POWER
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.parametricRimFresnelPower === undefined 
            && materialDefinition.model === "vrm_mtoon") {
        maMaterialData.parametricRimFresnelPowerIsActive = false;
        maParametricRimFresnelPowerIsActive.className = "uiMaterialAssistant-inactive";
        maParametricRimFresnelPower.disabled = true;
        maParametricRimFresnelPowerSlider.disabled = true;
    } else {
        maMaterialData.parametricRimFresnelPowerIsActive = true;
        maParametricRimFresnelPowerIsActive.className = "uiMaterialAssistant-active";
        maParametricRimFresnelPower.disabled = false;
        maParametricRimFresnelPowerSlider.disabled = false;
    } 

    if (materialDefinition.parametricRimFresnelPower !== undefined) {
        maMaterialData.parametricRimFresnelPower = materialDefinition.parametricRimFresnelPower;
    } else {
        maMaterialData.parametricRimFresnelPower = DEFAULT_PARAMETRIC_RIM_FRESNEL_POWER;
    }
    maParametricRimFresnelPower.value = maMaterialData.parametricRimFresnelPower;
    maParametricRimFresnelPowerSlider.value = Math.floor(maMaterialData.parametricRimFresnelPower * 1000);
    
    //PARAMETRIC RIM LIFT
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.parametricRimLift === undefined 
            && materialDefinition.model === "vrm_mtoon") {
        maMaterialData.parametricRimLiftIsActive = false;
        maParametricRimLiftIsActive.className = "uiMaterialAssistant-inactive";
        maParametricRimLift.disabled = true;
        maParametricRimLiftSlider.disabled = true;
    } else {
        maMaterialData.parametricRimLiftIsActive = true;
        maParametricRimLiftIsActive.className = "uiMaterialAssistant-active";
        maParametricRimLift.disabled = false;
        maParametricRimLiftSlider.disabled = false;
    } 

    if (materialDefinition.parametricRimLift !== undefined) {
        maMaterialData.parametricRimLift = materialDefinition.parametricRimLift;
    } else {
        maMaterialData.parametricRimLift = DEFAULT_PARAMETRIC_RIM_LIFT;
    }
    maParametricRimLift.value = maMaterialData.parametricRimLift;
    maParametricRimLiftSlider.value = Math.floor(maMaterialData.parametricRimLift * 1000);
    
/* ######## Not supported yet, but the code is ready #########################################
    //OUTLINE WIDTH MODE
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.outlineWidthMode === undefined 
            && materialDefinition.model === "vrm_mtoon") {
        maMaterialData.outlineWidthModeIsActive = false;
        maOutlineWidthModeIsActive.className = "uiMaterialAssistant-inactive";
        maOutlineWidthModeNone.disabled = true;
        maOutlineWidthModeWorld.disabled = true;
        maOutlineWidthModeScreen.disabled = true;
    } else {
        maMaterialData.outlineWidthModeIsActive = true;
        maOutlineWidthModeIsActive.className = "uiMaterialAssistant-active";
        maOutlineWidthModeNone.disabled = false;
        maOutlineWidthModeWorld.disabled = false;
        maOutlineWidthModeScreen.disabled = false;
    }
    
    if (materialDefinition.outlineWidthMode !== undefined) {
        maMaterialData.outlineWidthMode = materialDefinition.outlineWidthMode;
    } else {
        maMaterialData.outlineWidthMode = DEFAULT_OUTLINE_WIDTH_MODE;
    }

    switch (maMaterialData.outlineWidthMode) {
        case "none":
            maOutlineWidthModeNone.checked = true;
            break;
        case "worldCoordinates":
            maOutlineWidthModeWorld.checked = true;
            break;
        case "screenCoordinates":
            maOutlineWidthModeScreen.checked = true;
            break;
        default:
            alert("ERROR: Unrecognized material outlineWidthMode. (outlineWidthMode = '" + maMaterialData.outlineWidthMode + "')");
    }

    //OUTLINE WIDTH
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.outlineWidth === undefined 
            && materialDefinition.model === "vrm_mtoon") {
        maMaterialData.outlineWidthIsActive = false;
        maOutlineWidthIsActive.className = "uiMaterialAssistant-inactive";
        maOutlineWidth.disabled = true;
        maOutlineWidthSlider.disabled = true;
    } else {
        maMaterialData.outlineWidthIsActive = true;
        maOutlineWidthIsActive.className = "uiMaterialAssistant-active";
        maOutlineWidth.disabled = false;
        maOutlineWidthSlider.disabled = false;
    } 

    if (materialDefinition.outlineWidth !== undefined) {
        maMaterialData.outlineWidth = materialDefinition.outlineWidth;
    } else {
        maMaterialData.outlineWidth = DEFAULT_OUTLINE_WIDTH;
    }
    maOutlineWidth.value = maMaterialData.outlineWidth;
    maOutlineWidthSlider.value = Math.floor(maMaterialData.outlineWidth * 1000);

    //OUTLINE
    if (materialDefinition.defaultFallthrough === true && materialDefinition.outline === undefined
            && materialDefinition.model === "vrm_mtoon" ) {
        maMaterialData.outlineIsActive = false;
        maOutlineIsActive.className = "uiMaterialAssistant-inactive";
        maOutlineColorPicker.style.pointerEvents = 'none';
        maOutlineColorPicker.style.backgroundColor = "#555555";
    } else {
        maMaterialData.outlineIsActive = true;
        maOutlineIsActive.className = "uiMaterialAssistant-active";
        maOutlineColorPicker.style.pointerEvents = 'auto';
        maOutlineColorPicker.style.backgroundColor = maGetRGB(maMaterialData.outline);
    }        
    
    if (materialDefinition.outline !== undefined) {
        maMaterialData.outline = materialDefinition.outline;
    } else {
        maMaterialData.outline = DEFAULT_OUTLINE;
    }
    maOutlineColorPicker.style.backgroundColor = maGetRGB(maMaterialData.outline);
    
//##############################################################
*/

    //UV ANIMATION MASK MAP
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.uvAnimationMaskMap === undefined 
            && materialDefinition.model === "vrm_mtoon" ) {
        maMaterialData.uvAnimationMaskMapIsActive = false;
        maUvAnimationMaskMapIsActive.className = "uiMaterialAssistant-inactive";
        maUvAnimationMaskMap.disabled = true;
    } else {
        maMaterialData.uvAnimationMaskMapIsActive = true;
        maUvAnimationMaskMapIsActive.className = "uiMaterialAssistant-active";
        maUvAnimationMaskMap.disabled = false;
    } 

    if (materialDefinition.uvAnimationMaskMap !== undefined) {
        maMaterialData.uvAnimationMaskMap = materialDefinition.uvAnimationMaskMap;
    } else {
        maMaterialData.uvAnimationMaskMap = "";
    }
    maUvAnimationMaskMap.value = maMaterialData.uvAnimationMaskMap;

    //UV ANIMATION SCROLL X SPEED
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.uvAnimationScrollXSpeed === undefined 
            && materialDefinition.model === "vrm_mtoon") {
        maMaterialData.uvAnimationScrollXSpeedIsActive = false;
        maUvAnimationScrollXSpeedIsActive.className = "uiMaterialAssistant-inactive";
        maUvAnimationScrollXSpeed.disabled = true;
        maUvAnimationScrollXSpeedSlider.disabled = true;
    } else {
        maMaterialData.uvAnimationScrollXSpeedIsActive = true;
        maUvAnimationScrollXSpeedIsActive.className = "uiMaterialAssistant-active";
        maUvAnimationScrollXSpeed.disabled = false;
        maUvAnimationScrollXSpeedSlider.disabled = false;
    } 

    if (materialDefinition.uvAnimationScrollXSpeed !== undefined) {
        maMaterialData.uvAnimationScrollXSpeed = materialDefinition.uvAnimationScrollXSpeed;
    } else {
        maMaterialData.uvAnimationScrollXSpeed = DEFAULT_UV_ANIMATION_SCROLL_X_SPEED;
    }
    maUvAnimationScrollXSpeed.value = maMaterialData.uvAnimationScrollXSpeed;
    maUvAnimationScrollXSpeedSlider.value = Math.floor(maMaterialData.uvAnimationScrollXSpeed * 1000);

    //UV ANIMATION SCROLL Y SPEED
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.uvAnimationScrollYSpeed === undefined 
            && materialDefinition.model === "vrm_mtoon") {
        maMaterialData.uvAnimationScrollYSpeedIsActive = false;
        maUvAnimationScrollYSpeedIsActive.className = "uiMaterialAssistant-inactive";
        maUvAnimationScrollYSpeed.disabled = true;
        maUvAnimationScrollYSpeedSlider.disabled = true;
    } else {
        maMaterialData.uvAnimationScrollYSpeedIsActive = true;
        maUvAnimationScrollYSpeedIsActive.className = "uiMaterialAssistant-active";
        maUvAnimationScrollYSpeed.disabled = false;
        maUvAnimationScrollYSpeedSlider.disabled = false;
    } 

    if (materialDefinition.uvAnimationScrollYSpeed !== undefined) {
        maMaterialData.uvAnimationScrollYSpeed = materialDefinition.uvAnimationScrollYSpeed;
    } else {
        maMaterialData.uvAnimationScrollYSpeed = DEFAULT_UV_ANIMATION_SCROLL_Y_SPEED;
    }
    maUvAnimationScrollYSpeed.value = maMaterialData.uvAnimationScrollYSpeed;
    maUvAnimationScrollYSpeedSlider.value = Math.floor(maMaterialData.uvAnimationScrollYSpeed * 1000);

    //UV ANIMATION ROTATION SPEED
    if (materialDefinition.defaultFallthrough === true 
            && materialDefinition.uvAnimationRotationSpeed === undefined 
            && materialDefinition.model === "vrm_mtoon") {
        maMaterialData.uvAnimationRotationSpeedIsActive = false;
        maUvAnimationRotationSpeedIsActive.className = "uiMaterialAssistant-inactive";
        maUvAnimationRotationSpeed.disabled = true;
        maUvAnimationRotationSpeedSlider.disabled = true;
    } else {
        maMaterialData.uvAnimationRotationSpeedIsActive = true;
        maUvAnimationRotationSpeedIsActive.className = "uiMaterialAssistant-active";
        maUvAnimationRotationSpeed.disabled = false;
        maUvAnimationRotationSpeedSlider.disabled = false;
    } 

    if (materialDefinition.uvAnimationRotationSpeed !== undefined) {
        maMaterialData.uvAnimationRotationSpeed = materialDefinition.uvAnimationRotationSpeed;
    } else {
        maMaterialData.uvAnimationRotationSpeed = DEFAULT_UV_ANIMATION_ROTATION_SPEED;
    }
    maUvAnimationRotationSpeed.value = maMaterialData.uvAnimationRotationSpeed;
    maUvAnimationRotationSpeedSlider.value = Math.floor(maMaterialData.uvAnimationRotationSpeed * 1000);
    
    //PROCEDURAL
    if (materialDefinition.procedural !== undefined) {
        maMaterialData.procedural = JSON.stringify(materialDefinition.procedural, null, 4);
        maAddProceduralTemplate.disabled = true;
    } else {
        maMaterialData.procedural = "";
        maAddProceduralTemplate.disabled = false;
    }
    maProcedural.value = maMaterialData.procedural;
    
    //CULL FACE MODE
    if (materialDefinition.cullFaceMode !== undefined) {
        maMaterialData.cullFaceMode = materialDefinition.cullFaceMode;
    } else {
        maMaterialData.cullFaceMode = "CULL_BACK";
    }
    switch (maMaterialData.cullFaceMode) {
        case "CULL_BACK":
            maCullFaceModeBack.checked = true;
            break;
        case "CULL_FRONT":
            maCullFaceModeFront.checked = true;
            break;
        case "CULL_NONE":
            maCullFaceModeNone.checked = true;
            break;
        default:
            alert("ERROR: Unrecognized material cullFaceMode. (cullFaceMode = '" + maMaterialData.cullFaceMode + "')");
    }
    
    maFieldContextualDisplayer();
}

function maFieldContextualDisplayer() {
    if (maMaterialData.model === "hifi_pbr") {
        document.getElementById("maContainerAlbedo").style.display = "block";
        document.getElementById("maContainerAlbedoMap").style.display = "block";
        document.getElementById("maContainerMetallic").style.display = "block";
        document.getElementById("maContainerRoughness").style.display = "block";
        document.getElementById("maContainerNormalMap").style.display = "block";
        document.getElementById("maContainerOpacity").style.display = "block";
        document.getElementById("maContainerOpacityMap").style.display = "block";
        document.getElementById("maContainerEmissive").style.display = "block";
        document.getElementById("maContainerEmissiveMap").style.display = "block";
        document.getElementById("maContainerUnlit").style.display = "block";
        document.getElementById("maContainerScattering").style.display = "block";
        document.getElementById("maContainerOcclusionMap").style.display = "block";
        document.getElementById("maContainerLightMap").style.display = "block";
        document.getElementById("maContainerMtoon").style.display = "none";
        document.getElementById("maContainerShaderSimple").style.display = "none";
    } else if (maMaterialData.model === "vrm_mtoon") {
        document.getElementById("maContainerAlbedo").style.display = "block";
        document.getElementById("maContainerAlbedoMap").style.display = "block";
        document.getElementById("maContainerMetallic").style.display = "none";
        document.getElementById("maContainerRoughness").style.display = "none";
        document.getElementById("maContainerNormalMap").style.display = "block";
        document.getElementById("maContainerOpacity").style.display = "block";
        document.getElementById("maContainerOpacityMap").style.display = "block";
        document.getElementById("maContainerEmissive").style.display = "block";
        document.getElementById("maContainerEmissiveMap").style.display = "block";
        document.getElementById("maContainerUnlit").style.display = "none";
        document.getElementById("maContainerScattering").style.display = "none";
        document.getElementById("maContainerOcclusionMap").style.display = "none";
        document.getElementById("maContainerLightMap").style.display = "none";
        document.getElementById("maContainerMtoon").style.display = "block";
        document.getElementById("maContainerShaderSimple").style.display = "none";
    } else if (maMaterialData.model === "hifi_shader_simple") {
        document.getElementById("maContainerAlbedo").style.display = "block";
        document.getElementById("maContainerAlbedoMap").style.display = "none";
        document.getElementById("maContainerMetallic").style.display = "none";
        document.getElementById("maContainerRoughness").style.display = "none";
        document.getElementById("maContainerNormalMap").style.display = "none";
        document.getElementById("maContainerOpacity").style.display = "block";
        document.getElementById("maContainerOpacityMap").style.display = "none";
        document.getElementById("maContainerEmissive").style.display = "none";
        document.getElementById("maContainerEmissiveMap").style.display = "none";
        document.getElementById("maContainerUnlit").style.display = "none";
        document.getElementById("maContainerScattering").style.display = "none";
        document.getElementById("maContainerOcclusionMap").style.display = "none";
        document.getElementById("maContainerLightMap").style.display = "none";
        document.getElementById("maContainerMtoon").style.display = "none";
        document.getElementById("maContainerShaderSimple").style.display = "block";
    }
}

function maGenerateJsonAndSave() {
    var newMaterial = {};
    var defaultFallthrough = false;
    
    //MODEL
    newMaterial.model = maMaterialData.model;

    //NAME
    if (maMaterialData.name != "") {
        newMaterial.name = maMaterialData.name;
    }
    
    //ALBEDO
    if (maMaterialData.albedoIsActive) {
        newMaterial.albedo = maMaterialData.albedo;
    } else {
        defaultFallthrough = true;
    }
    

    if (maMaterialData.model === "hifi_pbr") {

        //ALBEDOMAP
        if (maMaterialData.albedoMapIsActive) {
                newMaterial.albedoMap = maMaterialData.albedoMap || "";
        } else {
            defaultFallthrough = true;
        }

        //METALLIC & METALLICMAP
        if (maMaterialData.metallicIsActive) {
            if (maMaterialData.metallicMap === "") {
                newMaterial.metallic = maMaterialData.metallic;
            } else {
                if (maMaterialData.useSpecular) {
                    newMaterial.specularMap = maMaterialData.metallicMap;
                } else {
                    newMaterial.metallicMap = maMaterialData.metallicMap;
                }
            }
        } else {
            defaultFallthrough = true;
        }
        
        //ROUGHNESS & ROUGHNESSMAP
        if (maMaterialData.roughnessIsActive) {
            if (maMaterialData.roughnessMap === "") {
                newMaterial.roughness = maMaterialData.roughness;
            } else {
                if (maMaterialData.useGloss) {
                    newMaterial.glossMap = maMaterialData.roughnessMap;
                } else {
                    newMaterial.roughnessMap = maMaterialData.roughnessMap;
                }
            }
        } else {
            defaultFallthrough = true;
        }

        //NORMAL MAP
        if (maMaterialData.normalMapIsActive) {
            if (maMaterialData.useBump) {
                newMaterial.bumpMap = maMaterialData.normalMap || "";
            } else {
                newMaterial.normalMap = maMaterialData.normalMap || "";
            }
        } else {
            defaultFallthrough = true;
        }

        //OPACITY
        if (maMaterialData.opacityIsActive) {
            switch (maMaterialData.opacityMapMode) {
                case "OPACITY_MAP_OPAQUE":
                    newMaterial.opacity = maMaterialData.opacity;
                    break;
                case "OPACITY_MAP_MASK":
                    newMaterial.opacityMapMode = maMaterialData.opacityMapMode;
                    newMaterial.opacityMap = maMaterialData.albedoMap;
                    newMaterial.opacityCutoff = maMaterialData.opacityCutoff;
                    break;
                case "OPACITY_MAP_BLEND":
                    newMaterial.opacityMapMode = maMaterialData.opacityMapMode;
                    newMaterial.opacityMap = maMaterialData.albedoMap;
                    break;
                default:
                    alert("ERROR: Unrecognized material opacityMapMode. (opacityMapMode = '" + maMaterialData.opacityMapMode + "')");
            }
        } else {
            defaultFallthrough = true;
        }

        //EMISSIVE
        if (maMaterialData.emissiveIsActive) {
            if (maMaterialData.emissiveMap === "") {
                newMaterial.emissive = scaleEmissiveByBloomFactor(maMaterialData.emissive, maMaterialData.bloom);
            } else {
                newMaterial.emissiveMap = maMaterialData.emissiveMap;
            }        
        } else {
            defaultFallthrough = true;
        } 

        //UNLIT
        if (maMaterialData.unlit) {
            newMaterial.unlit = maMaterialData.unlit;
        }

        //SCATTERING
        if (maMaterialData.scatteringIsActive) {
            if (maMaterialData.scatteringMap === "") {
                newMaterial.scattering = maMaterialData.scattering;
            } else {
                newMaterial.scatteringMap = maMaterialData.scatteringMap;
            }
        } else {
            defaultFallthrough = true;
        }
        
        //OCCLUSION MAP
        if (maMaterialData.occlusionMapIsActive) {
            newMaterial.occlusionMap = maMaterialData.occlusionMap || "";
        } else {
            defaultFallthrough = true;
        }

        //LIGHT MAP
        if (maMaterialData.lightMapIsActive) {
            newMaterial.lightMap = maMaterialData.lightMap || "";
        } else {
            defaultFallthrough = true;
        }

    } else if (maMaterialData.model === "vrm_mtoon") {
        //ALBEDOMAP
        if (maMaterialData.albedoMapIsActive) {
            newMaterial.albedoMap = maMaterialData.albedoMap || "";
        } else {
            defaultFallthrough = true;
        }

        //NORMAL MAP
        if (maMaterialData.normalMapIsActive) {
            if (maMaterialData.useBump) {
                newMaterial.bumpMap = maMaterialData.normalMap || "";
            } else {
                newMaterial.normalMap = maMaterialData.normalMap || "";
            }
        } else {
            defaultFallthrough = true;
        }
        
        //EMISSIVE
        if (maMaterialData.emissiveIsActive) {
            if (maMaterialData.emissiveMap === "") {
                newMaterial.emissive = scaleEmissiveByBloomFactor(maMaterialData.emissive, maMaterialData.bloom);
            } else {
                newMaterial.emissiveMap = maMaterialData.emissiveMap;
            }        
        } else {
            defaultFallthrough = true;
        }
        
        //OPACITY
        if (maMaterialData.opacityIsActive) {
            switch (maMaterialData.opacityMapMode) {
                case "OPACITY_MAP_OPAQUE":
                    newMaterial.opacity = maMaterialData.opacity;
                    break;
                case "OPACITY_MAP_MASK":
                    newMaterial.opacityMapMode = maMaterialData.opacityMapMode;
                    newMaterial.opacityMap = maMaterialData.albedoMap;
                    newMaterial.opacityCutoff = maMaterialData.opacityCutoff;
                    break;
                case "OPACITY_MAP_BLEND":
                    newMaterial.opacityMapMode = maMaterialData.opacityMapMode;
                    newMaterial.opacityMap = maMaterialData.albedoMap;
                    break;
                default:
                    alert("ERROR: Unrecognized material opacityMapMode. (opacityMapMode = '" + maMaterialData.opacityMapMode + "')");
            }
        } else {
            defaultFallthrough = true;
        }
        
        //SHADE
        if (maMaterialData.shadeIsActive) {
            newMaterial.shade = maMaterialData.shade;
        } else {
            defaultFallthrough = true;
        }
        
        //SHADEMAP
        if (maMaterialData.shadeMapIsActive) {
            newMaterial.shadeMap = maMaterialData.shadeMap || "";
        } else {
            defaultFallthrough = true;
        }

        //MATCAP
        if (maMaterialData.matcapIsActive) {
            newMaterial.matcap = maMaterialData.matcap;
        } else {
            defaultFallthrough = true;
        }
        
        //MATCAPMAP
        if (maMaterialData.matcapMapIsActive) {
            newMaterial.matcapMap = maMaterialData.matcapMap || "";
        } else {
            defaultFallthrough = true;
        }
        //SHADING SHIFT
        if (maMaterialData.shadingShiftIsActive) {
            newMaterial.shadingShift = maMaterialData.shadingShift;
        } else {
            defaultFallthrough = true;
        }
        
        //SHADING SHIFT MAP
        if (maMaterialData.shadingShiftMapIsActive) {
            newMaterial.shadingShiftMap = maMaterialData.shadingShiftMap || "";
        } else {
            defaultFallthrough = true;
        }

        //SHADING TOONY
        if (maMaterialData.shadingToonyIsActive) {
            newMaterial.shadingToony = maMaterialData.shadingToony;
        } else {
            defaultFallthrough = true;
        }

        //PARAMETRIC RIM
        if (maMaterialData.parametricRimIsActive) {
            newMaterial.parametricRim = maMaterialData.parametricRim;
        } else {
            defaultFallthrough = true;
        }
        
        //RIM MAP
        if (maMaterialData.rimMapIsActive) {
            newMaterial.rimMap = maMaterialData.rimMap || "";
        } else {
            defaultFallthrough = true;
        }
        
        //RIM LIGHTING MIX
        if (maMaterialData.rimLightingMixIsActive) {
            newMaterial.rimLightingMix = maMaterialData.rimLightingMix;
        } else {
            defaultFallthrough = true;
        }

        //PARAMETRIC RIM FRESNEL POWER
        if (maMaterialData.parametricRimFresnelPowerIsActive) {
            newMaterial.parametricRimFresnelPower = maMaterialData.parametricRimFresnelPower;
        } else {
            defaultFallthrough = true;
        }
        
        //PARAMETRIC RIM LIFT
        if (maMaterialData.parametricRimLiftIsActive) {
            newMaterial.parametricRimLift = maMaterialData.parametricRimLift;
        } else {
            defaultFallthrough = true;
        }
/* ###################### Not yet supported but ready for it... #############################

        //OUTLINE WIDTH MODE
        if (maMaterialData.outlineWidthModeIsActive) {
            newMaterial.outlineWidthMode = maMaterialData.outlineWidthMode;
        } else {
            defaultFallthrough = true;
        }
        
        //OUTLINE WIDTH
        if (maMaterialData.outlineWidthIsActive) {
            newMaterial.outlineWidth = maMaterialData.outlineWidth;
        } else {
            defaultFallthrough = true;
        }
        
        //OUTLINE
        if (maMaterialData.outlineIsActive) {
            newMaterial.outline = maMaterialData.outline;
        } else {
            defaultFallthrough = true;
        }
        
        if (newMaterial.outlineWidthMode === "none") {
            delete newMaterial.outlineWidth; 
            delete newMaterial.outline;
        }
        
        //#####################################################################################
*/
        //UV ANIMATION MASK MAP
        if (maMaterialData.uvAnimationMaskMapIsActive) {
            newMaterial.uvAnimationMaskMap = maMaterialData.uvAnimationMaskMap || "";
        } else {
            defaultFallthrough = true;
        }

        //UV ANIMATION SCROLL X SPEED
        if (maMaterialData.uvAnimationScrollXSpeedIsActive) {
            newMaterial.uvAnimationScrollXSpeed = maMaterialData.uvAnimationScrollXSpeed;
        } else {
            defaultFallthrough = true;
        }

        //UV ANIMATION SCROLL Y SPEED
        if (maMaterialData.uvAnimationScrollYSpeedIsActive) {
            newMaterial.uvAnimationScrollYSpeed = maMaterialData.uvAnimationScrollYSpeed;
        } else {
            defaultFallthrough = true;
        }

        //UV ANIMATION ROTATION SPEED
        if (maMaterialData.uvAnimationRotationSpeedIsActive) {
            newMaterial.uvAnimationRotationSpeed = maMaterialData.uvAnimationRotationSpeed;
        } else {
            defaultFallthrough = true;
        }
        
    } else if (maMaterialData.model === "hifi_shader_simple") {
        //OPACITY
        if (maMaterialData.opacityIsActive) {
            newMaterial.opacity = maMaterialData.opacity;
        } else {
            defaultFallthrough = true;
        }

        //PROCEDURAL
        
        if ( maMaterialData.procedural === "") {
            maAddProceduralTemplate.disabled = false;
        } else {
            newMaterial.procedural = JSON.parse(maMaterialData.procedural);
            maAddProceduralTemplate.disabled = true;
        }

    }

    //CULL FACE MODE
    newMaterial.cullFaceMode = maMaterialData.cullFaceMode;
    
    //defaultFallthrough
    if (defaultFallthrough) {
        newMaterial.defaultFallthrough = true;
    }

    //Remove the empty map when defaultFallthrough is false
    if (!defaultFallthrough) {
        if (newMaterial.albedoMap === "") {delete newMaterial.albedoMap; }
        if (newMaterial.normalMap === "") {delete newMaterial.normalMap; }
        if (newMaterial.bumpMap === "") {delete newMaterial.bumpMap; }
        if (newMaterial.occlusionMap === "") {delete newMaterial.occlusionMap; }
        if (newMaterial.lightMap === "") {delete newMaterial.lightMap; }
        if (newMaterial.shadeMap === "") {delete newMaterial.shadeMap; }
        if (newMaterial.matcapMap === "") {delete newMaterial.matcapMap; }
        if (newMaterial.shadingShiftMap === "") {delete newMaterial.shadingShiftMap; }
        if (newMaterial.rimMap === "") {delete newMaterial.rimMap; }
        if (newMaterial.uvAnimationMaskMap === "") {delete newMaterial.uvAnimationMaskMap; }
/* ############## Not yet supported, but the code is ready (commented) ###################
        if (newMaterial.outlineWidthMode === "none") {delete newMaterial.outlineWidthMode; }
//############################################################################################ */
    }

    //insert newMaterial to materialData
    var materialDataForUpdate = {
            "materialVersion": 1,
            "materials": []
        };
    materialDataForUpdate.materials.push(newMaterial);

    //save to property
    EventBridge.emitWebEvent(
            JSON.stringify({
                ids: [...selectedEntityIDs],
                type: "saveMaterialData",
                properties: {
                    materialData: JSON.stringify(materialDataForUpdate)
                }
            })
        );
    materialEditor.set(materialDataForUpdate);
}

function maGetColorValueFromEmissive(colorArray) {
    if (Array.isArray(colorArray)) {
        var max = maGetHighestValue(colorArray);
        if (max > 1) {
            var normalizer = 1/max;
            return [colorArray[0] * normalizer, colorArray[1] * normalizer, colorArray[2] * normalizer];
        } else {
            return colorArray;
        }
    } else {
        return [0,0,0];
    }
}

function maGetBloomFactorFromEmissive(colorArray) {
    if (Array.isArray(colorArray)) {
        return maGetHighestValue(colorArray);
    } else {
        return 1;
    }    
}

function maGetHighestValue(colorArray) {
    var highest = colorArray[0];
    for (var i = 0; i < colorArray.length; i++) {
        if (colorArray[i] > highest) {
            highest = colorArray[i];
        }        
    }
    return highest;
}

function scaleEmissiveByBloomFactor(colorArray, bloomFactor) {
    return [colorArray[0] * bloomFactor, colorArray[1] * bloomFactor, colorArray[2] * bloomFactor];
}

function maGetRGB(colorArray){
    if (colorArray === undefined) {
        return "#000000";
    } else {
        return "rgb(" + Math.floor(colorArray[0] * 255) + ", " + Math.floor(colorArray[1] * 255) + ", " + Math.floor(colorArray[2] * 255) + ")";
    }
}

/**
 * @param {string or object} materialData - json of the materialData as a string or as an object.
 */
function maGetMaterialDataAssistantAvailability(materialData) {
    var materialDataJSON, materialDataString;
    if (typeof materialData === "string") {
        materialDataJSON = JSON.parse(materialData);
        materialDataString = materialData;
    } else {
        materialDataJSON = materialData;
        materialDataString = JSON.stringify(materialData);
    }

    if (getPropertyInputElement("materialURL").value === "materialData" &&
            materialDataString.indexOf("outlineWidthMode") === -1 && //<=== ##### REMOVE THIS LINE WHEN YOU REACTIVATE OUTINE BLOCK (commented in the code) #####
            materialDataString.indexOf("outlineWidth") === -1 && //<=== ##### REMOVE THIS LINE WHEN YOU REACTIVATE OUTINE BLOCK (commented in the code) #####
            materialDataString.indexOf("outline") === -1 && //<=== ##### REMOVE THIS LINE WHEN YOU REACTIVATE OUTINE BLOCK (commented in the code) #####
            materialDataString.indexOf("texCoordTransform0") === -1 && 
            materialDataString.indexOf("texCoordTransform1") === -1 &&
            (materialDataJSON.materials === undefined || materialDataJSON.materials.length <= 1 || typeof materialDataJSON.materials === "object")) {
        showMaterialAssistantButton();
    } else {
        hideMaterialAssistantButton();
    }
}
