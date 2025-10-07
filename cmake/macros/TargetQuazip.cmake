# 
#  Copyright 2015 High Fidelity, Inc.
#  Created by Leonardo Murillo on 2015/11/20
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
# 
macro(TARGET_QUAZIP)
  # QT6TODO: if this is found twice, it throws an error. This will be called in init.cmake
  #find_package(QuaZip-Qt6 REQUIRED)
  target_link_libraries(${TARGET_NAME} QuaZip::QuaZip)
endmacro()
