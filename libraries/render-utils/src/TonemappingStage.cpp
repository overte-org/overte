//
//  TonemappingStage.cpp
//
//  Created by HifiExperiments on 6/24/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "TonemappingStage.h"

template <>
std::string render::PointerStage<graphics::Tonemapping, graphics::TonemappingPointer>::_name { "TONEMAPPING_STAGE" };
