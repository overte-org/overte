# Copyright 2013-2019, High Fidelity, Inc.
# Copyright 2021-2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME avatars-renderer)
setup_hifi_library(Network)
link_hifi_libraries(shared shaders gpu graphics animation material-networking model-networking render render-utils image entities-renderer physics recording)
include_hifi_library_headers(avatars)
include_hifi_library_headers(networking)
include_hifi_library_headers(hfm)
include_hifi_library_headers(model-serializers)
include_hifi_library_headers(ktx)
include_hifi_library_headers(procedural)
include_hifi_library_headers(audio)
include_hifi_library_headers(entities)
include_hifi_library_headers(octree)
include_hifi_library_headers(task)
include_hifi_library_headers(workload)
include_hifi_library_headers(graphics-scripting) # for ScriptableModel.h
include_hifi_library_headers(script-engine)

target_bullet()
