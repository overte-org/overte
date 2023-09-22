# 
#  Copyright 2015 High Fidelity, Inc.
#  Created by Bradley Austin Davis on 2015/10/10
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
# 
macro(TARGET_BULLET)
    if (ANDROID)
        set(INSTALL_DIR ${HIFI_ANDROID_PRECOMPILED}/bullet)
        set(BULLET_INCLUDE_DIRS "${INSTALL_DIR}/include/bullet" CACHE STRING INTERNAL)

        set(LIB_DIR ${INSTALL_DIR}/lib)
        list(APPEND BULLET_LIBRARIES ${LIB_DIR}/libBulletDynamics.a)
        list(APPEND BULLET_LIBRARIES ${LIB_DIR}/libBulletCollision.a)
        list(APPEND BULLET_LIBRARIES ${LIB_DIR}/libLinearMath.a)
        list(APPEND BULLET_LIBRARIES ${LIB_DIR}/libBulletSoftBody.a)
    else()
        find_package(Bullet REQUIRED)
   endif()
    target_link_libraries(${TARGET_NAME} Bullet::Bullet)
endmacro()


