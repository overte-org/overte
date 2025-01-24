# Conan automatically generated toolchain file
# DO NOT EDIT MANUALLY, it will be overwritten

# Avoid including toolchain file several times (bad if appending to variables like
#   CMAKE_CXX_FLAGS. See https://github.com/android/ndk/issues/323
include_guard()
message(STATUS "Using Conan toolchain: ${CMAKE_CURRENT_LIST_FILE}")
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeToolchain' generator only works with CMake >= 3.15")
endif()

########## 'user_toolchain' block #############
# Include one or more CMake user toolchain from tools.cmake.cmaketoolchain:user_toolchain



########## 'generic_system' block #############
# Definition of system, platform and toolset





########## 'compilers' block #############



########## 'arch_flags' block #############
# Define C++ flags, C flags and linker flags from 'settings.arch'

message(STATUS "Conan toolchain: Defining architecture flag: -m64")
string(APPEND CONAN_CXX_FLAGS " -m64")
string(APPEND CONAN_C_FLAGS " -m64")
string(APPEND CONAN_SHARED_LINKER_FLAGS " -m64")
string(APPEND CONAN_EXE_LINKER_FLAGS " -m64")


########## 'libcxx' block #############
# Definition of libcxx from 'compiler.libcxx' setting, defining the
# right CXX_FLAGS for that libcxx



########## 'cppstd' block #############
# Define the C++ and C standards from 'compiler.cppstd' and 'compiler.cstd'

function(conan_modify_std_watch variable access value current_list_file stack)
    set(conan_watched_std_variable "17")
    if (${variable} STREQUAL "CMAKE_C_STANDARD")
        set(conan_watched_std_variable "")
    endif()
    if ("${access}" STREQUAL "MODIFIED_ACCESS" AND NOT "${value}" STREQUAL "${conan_watched_std_variable}")
        message(STATUS "Warning: Standard ${variable} value defined in conan_toolchain.cmake to ${conan_watched_std_variable} has been modified to ${value} by ${current_list_file}")
    endif()
    unset(conan_watched_std_variable)
endfunction()

message(STATUS "Conan toolchain: C++ Standard 17 with extensions ON")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
variable_watch(CMAKE_CXX_STANDARD conan_modify_std_watch)


########## 'extra_flags' block #############
# Include extra C++, C and linker flags from configuration tools.build:<type>flags
# and from CMakeToolchain.extra_<type>_flags

# Conan conf flags start: 
# Conan conf flags end


########## 'cmake_flags_init' block #############
# Define CMAKE_<XXX>_FLAGS from CONAN_<XXX>_FLAGS

foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER ${config} config)
    if(DEFINED CONAN_CXX_FLAGS_${config})
      string(APPEND CMAKE_CXX_FLAGS_${config}_INIT " ${CONAN_CXX_FLAGS_${config}}")
    endif()
    if(DEFINED CONAN_C_FLAGS_${config})
      string(APPEND CMAKE_C_FLAGS_${config}_INIT " ${CONAN_C_FLAGS_${config}}")
    endif()
    if(DEFINED CONAN_SHARED_LINKER_FLAGS_${config})
      string(APPEND CMAKE_SHARED_LINKER_FLAGS_${config}_INIT " ${CONAN_SHARED_LINKER_FLAGS_${config}}")
    endif()
    if(DEFINED CONAN_EXE_LINKER_FLAGS_${config})
      string(APPEND CMAKE_EXE_LINKER_FLAGS_${config}_INIT " ${CONAN_EXE_LINKER_FLAGS_${config}}")
    endif()
endforeach()

if(DEFINED CONAN_CXX_FLAGS)
  string(APPEND CMAKE_CXX_FLAGS_INIT " ${CONAN_CXX_FLAGS}")
endif()
if(DEFINED CONAN_C_FLAGS)
  string(APPEND CMAKE_C_FLAGS_INIT " ${CONAN_C_FLAGS}")
endif()
if(DEFINED CONAN_SHARED_LINKER_FLAGS)
  string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT " ${CONAN_SHARED_LINKER_FLAGS}")
endif()
if(DEFINED CONAN_EXE_LINKER_FLAGS)
  string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " ${CONAN_EXE_LINKER_FLAGS}")
endif()


########## 'extra_variables' block #############
# Definition of extra CMake variables from tools.cmake.cmaketoolchain:extra_variables



########## 'try_compile' block #############
# Blocks after this one will not be added when running CMake try/checks

get_property( _CMAKE_IN_TRY_COMPILE GLOBAL PROPERTY IN_TRY_COMPILE )
if(_CMAKE_IN_TRY_COMPILE)
    message(STATUS "Running toolchain IN_TRY_COMPILE")
    return()
endif()


########## 'find_paths' block #############
# Define paths to find packages, programs, libraries, etc.
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/conan_cmakedeps_paths.cmake")
  message(STATUS "Conan toolchain: Including CMakeDeps generated conan_find_paths.cmake")
  include("${CMAKE_CURRENT_LIST_DIR}/conan_cmakedeps_paths.cmake")
else()

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# Definition of CMAKE_MODULE_PATH
list(PREPEND CMAKE_MODULE_PATH "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib/cmake/Qt5Quick" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib/cmake/Qt5Scxml" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib/cmake/Qt5Qml" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib/cmake/Qt5Widgets" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib/cmake/Qt5Gui" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/bin" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib/cmake/Qt5Core")
# the generators folder (where conan generates files, like this toolchain)
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# Definition of CMAKE_PREFIX_PATH, CMAKE_XXXXX_PATH
# The explicitly defined "builddirs" of "host" context dependencies must be in PREFIX_PATH
list(PREPEND CMAKE_PREFIX_PATH "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib/cmake/Qt5Quick" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib/cmake/Qt5Scxml" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib/cmake/Qt5Qml" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib/cmake/Qt5Widgets" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib/cmake/Qt5Gui" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/bin" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib/cmake/Qt5Core")
# The Conan local "generators" folder, where this toolchain is saved.
list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR} )
list(PREPEND CMAKE_LIBRARY_PATH "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/platforms" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/playlistformats" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/position" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/geoservices" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/imageformats" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/iconengines" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/sqldrivers" "lib" "lib" "/home/juliangro/.conan2/p/b/fontcc688c5c89a284/p/lib" "lib" "lib" "lib" "lib" "lib" "lib" "lib" "lib" "lib" "lib" "/home/juliangro/.conan2/p/b/odbcb745ba7f94832/p/lib" "/home/juliangro/.conan2/p/b/libto201aaf23b4bde/p/lib" "lib" "lib" "/home/juliangro/.conan2/p/b/xkbco223da449aafe3/p/lib" "lib" "lib" "lib" "lib" "lib" "lib" "lib" "/home/juliangro/.conan2/p/b/libxse5635a2079549/p/lib" "/home/juliangro/.conan2/p/b/nssb1d8c7def7e78/p/lib" "/home/juliangro/.conan2/p/b/nsprc5d1b95cc6e17/p/lib" "lib" "lib" "lib" "lib" "lib" "lib")
list(PREPEND CMAKE_INCLUDE_PATH "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtWebEngineWidgets" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtWebEngine" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtWebView" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtMultimediaQuick" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtWebEngineCore" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtLocation" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtQuickTemplates2" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtQuickControls2" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtQuickShapes" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtQuickWidgets" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtQuick" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtMultimediaWidgets" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtScxml" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtWebChannel" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtQmlWorkerScript" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtQmlModels" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtPrintSupport" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtXmlPatterns" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtWebSockets" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtMultimedia" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtSvg" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtQuickTest" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtQml" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtOpenGLExtensions" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtOpenGL" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtXkbCommonSupport" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtServiceSupport" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtWidgets" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtAccessibilitySupport" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtThemeSupport" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtFontDatabaseSupport" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtEventDispatcherSupport" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtPositioning" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtXml" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtConcurrent" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtTest" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtSql" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtNetwork" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtEdidSupport" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtGui" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/include/QtCore" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/mkspecs/linux-g++" "include" "include" "/home/juliangro/.conan2/p/b/fontcc688c5c89a284/p/include" "include" "include" "include" "include" "include" "include" "include" "include" "include" "include" "/home/juliangro/.conan2/p/b/odbcb745ba7f94832/p/include" "/home/juliangro/.conan2/p/b/libto201aaf23b4bde/p/include" "include" "include" "/home/juliangro/.conan2/p/b/xkbco223da449aafe3/p/include" "include" "include" "include" "include" "include" "include" "include" "/home/juliangro/.conan2/p/b/libxse5635a2079549/p/include" "/home/juliangro/.conan2/p/xorg-7088df65c658e/p/include" "/home/juliangro/.conan2/p/xorg-7088df65c658e/p/include/X11" "/home/juliangro/.conan2/p/b/nssb1d8c7def7e78/p/include" "/home/juliangro/.conan2/p/b/nssb1d8c7def7e78/p/include/nss" "/home/juliangro/.conan2/p/b/nsprc5d1b95cc6e17/p/include" "/home/juliangro/.conan2/p/b/nsprc5d1b95cc6e17/p/include/nspr" "include" "include" "include" "include" "include" "include")
set(CONAN_RUNTIME_LIB_DIRS "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/lib" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/platforms" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/playlistformats" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/position" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/geoservices" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/imageformats" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/iconengines" "/home/juliangro/.conan2/p/b/qt23376165b3dee/p/plugins/sqldrivers" "lib" "lib" "/home/juliangro/.conan2/p/b/fontcc688c5c89a284/p/lib" "lib" "lib" "lib" "lib" "lib" "lib" "lib" "lib" "lib" "lib" "/home/juliangro/.conan2/p/b/odbcb745ba7f94832/p/lib" "/home/juliangro/.conan2/p/b/libto201aaf23b4bde/p/lib" "lib" "lib" "/home/juliangro/.conan2/p/b/xkbco223da449aafe3/p/lib" "lib" "lib" "lib" "lib" "lib" "lib" "lib" "/home/juliangro/.conan2/p/b/libxse5635a2079549/p/lib" "/home/juliangro/.conan2/p/b/nssb1d8c7def7e78/p/lib" "/home/juliangro/.conan2/p/b/nsprc5d1b95cc6e17/p/lib" "lib" "lib" "lib" "lib" "lib" "lib" )

endif()


########## 'pkg_config' block #############
# Define pkg-config from 'tools.gnu:pkg_config' executable and paths

if (DEFINED ENV{PKG_CONFIG_PATH})
set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_LIST_DIR}:$ENV{PKG_CONFIG_PATH}")
else()
set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_LIST_DIR}:")
endif()


########## 'rpath' block #############
# Defining CMAKE_SKIP_RPATH



########## 'output_dirs' block #############
# Definition of CMAKE_INSTALL_XXX folders

set(CMAKE_INSTALL_BINDIR "bin")
set(CMAKE_INSTALL_SBINDIR "bin")
set(CMAKE_INSTALL_LIBEXECDIR "bin")
set(CMAKE_INSTALL_LIBDIR "lib")
set(CMAKE_INSTALL_INCLUDEDIR "include")
set(CMAKE_INSTALL_OLDINCLUDEDIR "include")


########## 'variables' block #############
# Definition of CMake variables from CMakeToolchain.variables values

# Variables
# Variables  per configuration



########## 'preprocessor' block #############
# Preprocessor definitions from CMakeToolchain.preprocessor_definitions values

# Preprocessor definitions per configuration



if(CMAKE_POLICY_DEFAULT_CMP0091)  # Avoid unused and not-initialized warnings
endif()
