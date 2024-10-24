# Copyright 2013-2019, High Fidelity, Inc.
# Copyright 2021-2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME physics)
setup_hifi_library()
link_hifi_libraries(shared workload entities shaders)
include_hifi_library_headers(audio)
include_hifi_library_headers(networking)
include_hifi_library_headers(gpu)
include_hifi_library_headers(avatars)
include_hifi_library_headers(octree)
include_hifi_library_headers(material-networking)
include_hifi_library_headers(model-networking)
include_hifi_library_headers(procedural)
include_hifi_library_headers(image)
include_hifi_library_headers(ktx)
include_hifi_library_headers(gpu)
include_hifi_library_headers(hfm)
include_hifi_library_headers(model-serializers)
include_hifi_library_headers(graphics)
include_hifi_library_headers(script-engine)
include_hifi_library_headers(entities)

target_bullet()
