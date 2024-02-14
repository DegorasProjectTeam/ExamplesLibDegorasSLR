# ----------------------------------------------------------------------------------------------------------------------
# INITIALIZATION

# Cmake minimum and empty project for avoid warnings.
cmake_minimum_required(VERSION 3.21)
project(_)

# Set path to additional CMake modules.
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})
include(CMakeMacrosGlobals)

# Init CMake modules.
macro_cmakemodules_init()

# ----------------------------------------------------------------------------------------------------------------------
# BASIC PROJECT CONFIGURATION

# Base project configuration.
set(PROJ_NAME ExamplesLibDegorasSLR)
set(PROJ_VER 1.9.0)
set(PROJ_BUILD_MODE Debug)
set(PROJ_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../${PROJ_NAME}_product_v${PROJ_VER})
set(PROJ_LANGUAGES CXX C)

# Macros global configuration.
macro_global_set_show_externals(TRUE)
macro_global_set_force_install_dir(TRUE)
macro_global_set_install_runtime_deps(TRUE)
macro_global_set_install_runtime_artifacts(TRUE)
macro_global_set_exclude_so_libraries(TRUE)

# Setup the base project.
macro_setup_base_project(${PROJ_NAME} ${PROJ_VER} ${PROJ_BUILD_MODE} ${PROJ_LANGUAGES})

# Other configurations.
set(CMAKE_CXX_STANDARD 17)

# Setup the install directory for each compiler and operating system.
macro_setup_default_install_dir(${PROJ_INSTALL_DIR})

# ----------------------------------------------------------------------------------------------------------------------
# SPECIFIC PROJECT CONFIGURATION

# Specific configuration.
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
set(LIB_INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes/)
set(LIB_SOURCES_DIR ${CMAKE_SOURCE_DIR}/sources)

# Configure the LibDegorasSLR package.
include(ConfigLibDegorasSLR)
macro_configure_libdegorasslr_default("1.9.0" "" "" "")

# ----------------------------------------------------------------------------------------------------------------------
# SETUP THE EXAMPLE 1

# Config.
set(EXAMPLE_NAME UtilitiesSLR_TrackingSLR)
set(EXAMPLE_SOURCES ${CMAKE_SOURCE_DIR}/UtilitiesSLR_TrackingSLR.cpp)

# Setup the launcher.
macro_setup_launcher("${EXAMPLE_NAME}"
                     "${MODULES_GLOBAL_LIBS_OPTIMIZED}"
                     "${MODULES_GLOBAL_LIBS_DEBUG}"
                     "${EXAMPLE_SOURCES}")

# Link with LibNovasCPP.
#macro_link_libnovascpp_default(${LIB_NAME})

target_link_libraries(${EXAMPLE_NAME} PUBLIC LibDegorasSLR::LibDegorasSLR)

get_target_property(libdegorasslr_includes LibDegorasSLR::LibDegorasSLR INTERFACE_INCLUDE_DIRECTORIES)
file(GLOB_RECURSE EXTERNAL_HEADERS ${libdegorasslr_includes}/*)
target_sources(${EXAMPLE_NAME} PRIVATE ${EXTERNAL_HEADERS})


# ----------------------------------------------------------------------------------------------------------------------
# CONFIGURE THE COMPILERS

# Configure the compilers.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -m64 -O3 -pthread -fopenmp")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    #target_compile_options(${LIB_NAME} PUBLIC)
    message(FATAL_ERROR "Compiler not supported by default.")
else()
    message(FATAL_ERROR "Compiler not supported by default.")
endif()



# ----------------------------------------------------------------------------------------------------------------------
# INSTALLATION PROCESS

# Installation process.
install (DIRECTORY resources DESTINATION ${CMAKE_BINARY_DIR})
# ----------------------------------------------------------------------------------------------------------------------
