#
#  Copyright 2015 High Fidelity, Inc.
#  Copyright 2025 Overte e.V.
#  Created by Leonardo Murillo on 2015/11/20
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#
macro(TARGET_QUAZIP)
    # System QuaZip sets `IMPORTED_GLOBAL` for itself when finding it using find_package().
    # Running find_package() a second time shouldn't do anything, since the package was already found,
    # yet it fails with:
    # `Attempt to promote imported target "QuaZip::QuaZip" to global scope (by setting IMPORTED_GLOBAL) which is not built in this directory.`
    # We avoid this error by guarding against running find_package() multiple times.
    if(OVERTE_USE_SYSTEM_LIBS)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(QuaZip REQUIRED quazip1-qt6)
        target_include_directories(${TARGET_NAME} SYSTEM PRIVATE ${QuaZip_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} ${QuaZip_LINK_LIBRARIES})
    else()
    	if(NOT TARGET QuaZip::QuaZip)  # if target doesn't exist.
        	find_package(QuaZip-Qt6 REQUIRED)
    	endif()
    	target_link_libraries(${TARGET_NAME} QuaZip::QuaZip)
    endif()
endmacro()
