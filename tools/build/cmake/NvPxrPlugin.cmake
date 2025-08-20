# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#

#[===============================================================[.rst:
NvPxrPlugin
-----------

Provides utility methods for building OpenUSD schemas and plugins.
This includes:
* Generating C++ code and resource files for codeful schema modules
* Optionally generating C++ code for python module bindings
* Building C++ plugins
* Building the Python bindings module
* Configuring the plugInfo.json file

Python is required to use this module, as schema generation and plugin
builds require the python interpreter, python headers, and python libraries
to be present.  This module is informed where the compatible version of
Python is for the OpenUSD library you are using by setting the following
cmake variables:
* PXR_OPENUSD_PYTHON_DIR: The directory where the python executable resides

Imported Targets
^^^^^^^^^^^^^^^^

This module will import targets from `pxrConfig.cmake` using find_package.
Ensure that your CMAKE_PREFIX_PATH includes the location of the `pxrConfig.cmake`
file from your OpenUSD dependency.

This module will find Python given the path provided in PXR_OPENUSD_PYTHON_DIR
and will set all of the targets and variables expected from find_package(Python3).

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables my be set if schema generation is invoked:

``PXR_${NAME}_SCHEMA_INPUT_TIMESTAMP``
  The timestamp of the schema input file associated with the plugin
  with the given NAME.

#]===============================================================]

set(USD_PLUGIN_CMAKE_UTILS_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

if(NOT PXR_OPENUSD_PYTHON_DIR)
    message(FATAL_ERROR "Must provide value for PXR_OPENUSD_PYTHON_DIR!")
endif()

# configure cmake find such that it will locate python
# from the specific directory we give it, unfortunately
# it searches some other places before the hints so need
# to configure those to come later in the sequence
# also unfortunately some of our OpenUSD packages contain
# duplicate python headers, so it may find the wrong
# include directory if we don't specify it because
# of cmake paths and the fact that using modules
# we have no control over limitation of NO_CMAKE_PATH
set(Python3_ROOT_DIR "${PXR_OPENUSD_PYTHON_DIR}")
if (WIN32)
    set(Python3_INCLUDE_DIR "${PXR_OPENUSD_PYTHON_DIR}/include")
else()
    # linux python packages have an extra level of indirection
    # on the include directory with the python version
    file(GLOB python_include_location "${PXR_OPENUSD_PYTHON_DIR}/include/python*")
    if (python_include_location)
        set(Python3_INCLUDE_DIR "${python_include_location}")
    else()
        message(FATAL_ERROR "Unable to determine python include directory under ${PXR_OPENUSD_PYTHON_DIR}")
    endif()
endif()
set(Python3_FIND_STRATEGY "LOCATION")
set(Python3_FIND_VIRTUALENV "NEVER")
set(Python3_FIND_FRAMEWORK "LAST")
set(Python3_FIND_REGISTRY "NEVER")
find_package(Python3
    REQUIRED
    COMPONENTS Interpreter Development)

# use the interpreter to get some values we need for building python modules
execute_process(COMMAND
    "${Python3_EXECUTABLE}"
    "-c"
    "
import sys
import sysconfig
is_supported = sys.version_info >= (3, 10)
if not is_supported:
    sys.exit(1)
print(sysconfig.get_config_var('EXT_SUFFIX'))
"
    RESULT_VARIABLE PYTHON_RETURN_CODE
    OUTPUT_VARIABLE PYTHON_PRINT_VALUES)

if(NOT PYTHON_RETURN_CODE MATCHES 0)
    message(FATAL_ERROR "Unsupported version of Python, must be >= 3.10!")
endif()

# this value holds the platform suffix plus extension
# we want to break those into two different values for use later
string(REGEX REPLACE "\n" "" PYTHON_PRINT_VALUES ${PYTHON_PRINT_VALUES})
set(PYTHON_TAG_SUFFIX_EXTENSION ${PYTHON_PRINT_VALUES})
string(FIND ${PYTHON_TAG_SUFFIX_EXTENSION} "." result REVERSE)
if(result EQUAL -1)
    message(FATAL_ERROR "Unable to retrieve extension from full Python extension suffix")
endif()
string(SUBSTRING ${PYTHON_TAG_SUFFIX_EXTENSION} 0 ${result} PYTHON_TAG_SUFFIX)
string(SUBSTRING ${PYTHON_TAG_SUFFIX_EXTENSION} ${result} -1 PYTHON_MODULE_EXTENSION)

# when importing pxrConfig.cmake, it will attempt to find python again
# since this doesn't seem to be completely reentrant, we also make sure
# to set the Python3_LIBRARY variable such that it should find everything
# we already found because everything is set appropriately
set(Python3_LIBRARY ${Python3_LIBRARIES})

# the pxrConfig.cmake file generated from OpenUSD will require
# policy CMP0012 to be set to properly recognize boolean constants
# we also set CMP0074 to NEW such that the user can specify search
# paths for the config files as <PackageName>_ROOT, which will
# be searched first in the new policy, prior to the cmake prefix path
# (unfortunately the cmake prefix path is searched before <PackageName>_DIR)
# so to give the user a chance to override we need to turn on CMP00074
cmake_policy(SET CMP0012 NEW)
cmake_policy(SET CMP0074 NEW)
find_package(pxr REQUIRED)

function (get_openusd_info)
    #[============================================================[.rst:
    get_openusd_info
    ---------------
    Retrieves information about the OpenUSD installation found.

    Inputs
    ^^^^^^
    This method requires the following cmake variables to be defined:
    * PXR_CMAKE_DIR: The root directory of the OpenUSD installation.
      This variable is typically defined in the pxrConfig.cmake file
      and imported via the find_package(pxr REQUIRED) statement. 
    
    Outputs
    ^^^^^^^
    This method will set the following cmake variables if they aren't
    set:
    * PXR_MAJOR_VERSION
    * PXR_MINOR_VERSION
    * PXR_PATCH_VERSION

    #]============================================================]

    if(NOT DEFINED PXR_MINOR_VERSION AND NOT DEFINED PXR_PATCH_VERSION)
        # attempt to find the version of OpenUSD being used
        set(PXR_H_FILE ${PXR_CMAKE_DIR}/include/pxr/pxr.h)
        file(STRINGS ${PXR_H_FILE} PXR_H_FILE_LINES)
        foreach(line ${PXR_H_FILE_LINES})
            string(FIND "${line}" "PXR_MAJOR_VERSION" result)
            if(NOT result EQUAL -1)
                # found major version
                math(EXPR result "${result} + 18")
                string(SUBSTRING ${line} ${result} -1 line)
                set(PXR_MAJOR_VERSION ${line} PARENT_SCOPE)
                continue()
            endif()
            string(FIND "${line}" "PXR_MINOR_VERSION" result)
            if(NOT result EQUAL -1)
                # found minor version
                math(EXPR result "${result} + 18")
                string(SUBSTRING ${line} ${result} -1 line)
                set(PXR_MINOR_VERSION ${line} PARENT_SCOPE)
                continue()
            endif()
            string(FIND "${line}" "PXR_PATCH_VERSION" result)
            if(NOT result EQUAL -1)
                # found patch version
                math(EXPR result "${result} + 18")
                string(SUBSTRING ${line} ${result} -1 line)
                set(PXR_PATCH_VERSION ${line} PARENT_SCOPE)
                break()
            endif()
        endforeach()
    endif()

endfunction()

function (setup_boost_python_info)

    # boost is potentially required if using any OpenUSD version prior to 24.11
    # boost python was the only boost library required as of 24.08
    # prior to 24.08, we have to setup targets for all of the boost
    # libraries that could potentially be used

    if (PXR_VERSION LESS_EQUAL 2408)
        # attempt to import boost using standard CMake scripts if one is available
        find_package(Boost QUIET CONFIG)

        # at least boost python was always required at this point
        # so we can check for the existence of the boost::pythonx target
        # to see if the find_package call was successful
        set(PYTHON_MAJOR_MINOR_VERSION "${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")
        if (NOT TARGET boost::python${PYTHON_MAJOR_MINOR_VERSION})
            # find_package was unsuccessful, but we have enough information
            # to set the basic boost targets that we will need to build
            # OpenUSD plugins
            
            # find the boost include directory
            # in most cases this will be ${PXR_CMAKE_DIR}/include
            # but older packages have an extra indirection for boost
            # such that the include directory will be something like
            # ${PXR_CMAKE_DIR}/include/boost-x.yz
            find_file(boost_version_h_found "version.hpp"
                PATHS "${PXR_CMAKE_DIR}/include/boost"
                NO_DEFAULT_PATH)
            if (NOT boost_version_h_found)
                # check for the extra level of indirection
                file(GLOB boost_include_location "${PXR_CMAKE_DIR}/include/boost*")
                if (boost_include_location)
                    set(BOOST_INCLUDE_DIRS "${boost_include_location}")
                else()
                    message(FATAL_ERROR "Unable to determine boost include directory under ${PXR_CMAKE_DIR}")
                endif()
            else()
                set(BOOST_INCLUDE_DIRS "${PXR_CMAKE_DIR}/include")
            endif()

            # the boost libraries are always under the lib directory
            set(BOOST_LIBRARY_DIRS "${PXR_CMAKE_DIR}/lib")

            # retrieve version information about boost
            file(STRINGS
                "${BOOST_INCLUDE_DIRS}/boost/version.hpp"
                boost_version
                REGEX "^#define[ \t]+BOOST_VERSION[ \t]+[0-9]+")
            string(REGEX REPLACE
                "^#define[ \t]+BOOST_VERSION[ \t]+([0-9]+).*"
                "\\1"
                boost_version "${boost_version}")
            math(EXPR Boost_VERSION_PATCH "${boost_version} % 100")
            math(EXPR Boost_VERSION_MINOR "${boost_version} / 100 % 1000")
            math(EXPR Boost_VERSION_MAJOR "${boost_version} / 100000")
            set(Boost_MAJOR_VERSION ${BOOST_VERSION_MAJOR})
            set(Boost_MINOR_VERSION ${BOOST_VERSION_MINOR})
            set(Boost_SUBMINOR_VERSION ${Boost_VERSION_PATCH})
            set(Boost_VERSION_MACRO ${boost_version})
            set(Boost_VERSION "${Boost_VERSION_MAJOR}.${Boost_VERSION_MINOR}.${Boost_VERSION_PATCH}")
            set(Boost_VERSION_STRING "${Boost_VERSION}")
            set(Boost_VERSION_COUNT 3)
            if (Boost_VERSION_PATCH EQUAL 0)
                set(Boost_LIB_VERSION "${Boost_VERSION_MAJOR}_${Boost_VERSION_MINOR}")
            else()
                set(Boost_LIB_VERSION "${Boost_VERSION_MAJOR}_${Boost_VERSION_MINOR}_${Boost_VERSION_PATCH}")
            endif()

            # create the necessary targets, this includes:
            # Boost::boost
            # Boost::headers
            # Boost::atomic
            # Boost::chrono
            # Boost::container
            # Boost::date_time
            # Boost::filesystem
            # Boost::program_options
            # Boost::python
            # boost::pythonXY
            # Boost::regex
            # Boost::system
            # Boost::thread
            # Boost::diagnostic_definitions
            # Boost::disable_autolinking
            # Boost::dynamic_linking
            add_library(Boost::headers INTERFACE IMPORTED)
            set_target_properties(Boost::headers PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")
            add_library(Boost::boost INTERFACE IMPORTED)
            set_target_properties(Boost::boost
                PROPERTIES INTERFACE_LINK_LIBRARIES Boost::headers)
            if (PXR_VERSION LESS 2408)
                list(APPEND boost_components atomic chrono container date_time filesystem program_options python${PYTHON_MAJOR_MINOR_VERSION} regex system thread)
            else()
                # will need to revise this for 25.02, which eliminates the boost depdency entirely
                list(APPEND boost_components python${PYTHON_MAJOR_MINOR_VERSION})
            endif()

            # set up the boost tags so we get the right library names
            if (WIN32)
                set(lib_prefix "")
                set(lib_suffix ".dll")
                set(import_lib_suffix ".lib")
                set(debug_abi_tag "-gd")
                set(runtime "-mt")
                set(boost_tag "-${Boost_LIB_VERSION}")
            else()
                set(lib_prefix "lib")
                
                # NOTE: The vendored linux OpenUSD libraries never have
                # the compiler or debug abi tags, for now we just assume
                # they always don't, but in more complex cases, like if it
                # finds certain targets on the user's machine, we may have
                # to be a little more sophisticated in how we handle this
                #set(debug_abi_tag "-d")
                set(debug_abi_tag "")
                set(lib_suffix ".so")
                set(import_lib_suffix ".so")
                set(runtime "")
                set(boost_tag "")
            endif()

            if ("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xMSVC")
                if (MSVC_TOOLSET_VERSION GREATER_EQUAL 140)
                    foreach (v 9 8 7 6 5 4 3 2 1 0)
                        if (MSVC_TOOLSET_VERSION GREATER_EQUAL 14${v})
                            set(compiler "-vc14${v}")
                            break()
                        endif()
                    endforeach()
                endif()
            elseif(WIN32)
                if ("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xClang")
                    string(REPLACE "." ";" VERSION_LIST "${CMAKE_CXX_COMPILER_VERSION}")
                    list(GET VERSION_LIST 0 CLANG_VERSION_MAJOR)
                    set(compiler "-clangw${CLANG_VERSION_MAJOR}")
                endif()
            else()
                string(REGEX REPLACE "([0-9]+)\\.([0-9]+)(\\.[0-9]+)?" "\\1"
                    compiler_version_major "${CMAKE_CXX_COMPILER_VERSION}")
                string(REGEX REPLACE "([0-9]+)\\.([0-9]+)(\\.[0-9]+)?" "\\2"
                    compiler_version_minor "${CMAKE_CXX_COMPILER_VERSION}")
                set(compiler_version "${compiler_version_major}${compiler_version_minor}")
                if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND compiler_version_major VERSION_GREATER 4)
                    set(compiler_version ${compiler_version_major})
                elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND compiler_version_major VERSION_GREATER 3)
                    set(compiler_version ${compiler_version_major})
                endif()
                if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
                    if(APPLE)
                        set(compiler "-xgcc${compiler_version}")
                    else()
                        set (compiler "")
                        #set(compiler "gcc${compiler_version}")
                    endif()
                elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
                    set(compiler "-clang${compiler_version}")
                endif()
            endif()

            if(CMAKE_CXX_COMPILER_ARCHITECTURE_ID MATCHES "^ARM")
                set(architecture "-a")
            else()
                if(WIN32)
                    set(architecture "-x64")
                else()
                    set(architecture "")
                endif()
            endif()

            set(boost_python_lib_suffix "python${PYTHON_MAJOR_MINOR_VERSION}")
            foreach(component ${boost_components})
                if (component STREQUAL boost_python_lib_suffix)
                    # the specifically versioned python component
                    # is always lower cased and a root Boost::Python target created
                    # to point to it
                    set(component_name boost::${component})
                else()
                    set(component_name Boost::${component})
                endif()
                add_library(${component_name} SHARED IMPORTED)
                set_target_properties(${component_name} PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")
                
                set(component_release_library_name "${lib_prefix}boost_${component}${compiler}${runtime}${architecture}${boost_tag}${lib_suffix}")
                set(component_release_import_library_name "${lib_prefix}boost_${component}${compiler}${runtime}${architecture}${boost_tag}${import_lib_suffix}")
                set(component_debug_library_name "${lib_prefix}boost_${component}${compiler}${runtime}${debug_abi_tag}${architecture}${boost_tag}${lib_suffix}")
                set(component_debug_import_library_name "${lib_prefix}boost_${component}${compiler}${runtime}${debug_abi_tag}${architecture}${boost_tag}${import_lib_suffix}")
                set_target_properties(${component_name} PROPERTIES
                    IMPORTED_CONFIGURATIONS "Debug;Release")
                set_target_properties(${component_name} PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                    IMPORTED_IMPLIB ${BOOST_LIBRARY_DIRS}/${component_release_import_library_name}
                    IMPORTED_LOCATION ${BOOST_LIBRARY_DIRS}/${component_release_library_name})
                set_target_properties(${component_name} PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
                    IMPORTED_IMPLIB_RELEASE ${BOOST_LIBRARY_DIRS}/${component_release_import_library_name}
                    IMPORTED_LOCATION_RELEASE ${BOOST_LIBRARY_DIRS}/${component_release_library_name})
                set_target_properties(${component_name} PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
                    IMPORTED_IMPLIB_DEBUG ${BOOST_LIBRARY_DIRS}/${component_debug_import_library_name}
                    IMPORTED_LOCATION_DEBUG ${BOOST_LIBRARY_DIRS}/${component_debug_library_name})

                if (component STREQUAL boost_python_lib_suffix)
                    add_library(Boost::Python INTERFACE IMPORTED)
                    target_link_libraries(Boost::Python INTERFACE ${component_name})
                endif()
            endforeach()

            add_library(Boost::diagnostic_definitions INTERFACE IMPORTED)
            add_library(Boost::disable_autolinking INTERFACE IMPORTED)
            add_library(Boost::dynamic_linking INTERFACE IMPORTED)
            set_target_properties(Boost::dynamic_linking PROPERTIES
                INTERFACE_COMPILE_DEFINITIONS "BOOST_ALL_DYN_LINK")
            if (WIN32)
                set(Boost_LIB_DIAGNOSTIC_DEFINITIONS "-DBOOST_LIB_DIAGNOSTIC")
                set_target_properties(Boost::diagnostic_definitions PROPERTIES
                    INTERFACE_COMPILE_DEFINITIONS "BOOST_LIB_DIAGNOSTIC")
                set_target_properties(Boost::disable_autolinking PROPERTIES
                    INTERFACE_COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB")
            endif()
            
            set(Boost_FOUND True)
        endif()
    endif()
endfunction()

# make sure we can locate and define targets for boost::python
setup_boost_python_info()

function (add_standard_openusd_options NAME)
    #[============================================================[.rst:
    add_standard_openusd_options
    ---------------
    Helper function to add a standard set of compile / link
    options to OpenUSD plugins.

    Inputs
    ^^^^^^
    * NAME: The name of the target to which the compiler options
      will be added.

    #]============================================================]
    if(MSVC)
        # The /Zc:inline option strips out the "arch_ctor_<name>" symbols used for
        # library initialization by ARCH_CONSTRUCTOR starting in Visual Studio 2019, 
        # causing release builds to fail. Disable the option for this and later 
        # versions.
        # 
        # For more details, see:
        # https://developercommunity.visualstudio.com/content/problem/914943/zcinline-removes-extern-symbols-inside-anonymous-n.html
        if (MSVC_VERSION GREATER_EQUAL 1920)
            target_compile_options(${NAME} PRIVATE "/Zc:inline-")
        else()
            target_compile_options(${NAME} PRIVATE "Zc:inline")
        endif()

        # turn off min/max macros defined in windows.h
        # exclude unecessary windows API headers
        target_compile_definitions(${NAME}
            PRIVATE
                "NOMINMAX"
                "WIN32_LEAN_AND_MEAN")

        # exception handling must be on
        # ensure standards conformance
        # setting /permissive- will set the following options to conforming behavior
        # /Zc:referenceBinding
        # /Zc:strictStrings
        # /Zc:rvalueCast
        # /Zc:ternary
        # set pdb format (not using edit and continue
        # so no need to set /Gy)
        # enable security checks
        # treat wchar_t as a built-in type
        target_compile_options(${NAME}
            PRIVATE
                "/EHsc"
                "/permissive-"
                "/Zi"
                "/GS"
                "/sdl"
                "/Zc:wchar_t")
    else()
        target_compile_options(${NAME}
            PRIVATE
                "-Wno-deprecated"
                "-Wno-deprecated-declarations"
                "-Wall"
                "-Wformat-security")

        get_target_property(target_type ${NAME} TYPE)
        if (target_type STREQUAL SHARED_LIBRARY)
            target_compile_options(${NAME}
                PRIVATE
                    "-fPIC")
        endif()
    endif()

    # if we are <= than 24.08
    # enable boost dynamic linking
    if (PXR_VERSION LESS_EQUAL 2408)
        target_link_libraries(${NAME} PRIVATE Boost::dynamic_linking)
    endif()

endfunction()

function (openusd_link_with_python NAME)
    #[============================================================[.rst:
    openusd_link_with_python
    ---------------
    Helper function to link the target with the supplied ${NAME}
    to Python if the OpenUSD libraries were built with Python enabled.
    If the OpenUSD libraries were not built with Python enabled, this
    function is a no-op.

    Inputs
    ^^^^^^
    * NAME: The name of the target to which Python will be linked
      if necessary.

    Targets
    ^^^^^^^
    If PXR_PYTHON_ENABLED=1 for a PXR target build, the target
    with the name ${NAME} will have Python3::Python linked as a
    target, as it will be needed transitively when building against
    the OpenUSD libraries.

    #]============================================================]

    # prior to OpenUSD 23.02, it was possible to check if OpenUSD had built
    # with python enabled by looking at the INTERFACE_COMPILE_DEFINITIONS
    # of the tf target.  These were removed in OpenUSD 23.02+, so we use
    # a heuristic to determine if python was used to build (because if it was
    # python may be required even for the C++ library when using tf) by
    # checking for the presence of the lib/python directory, since that's where
    # the OpenUSD python modules would be deployed to
    if (EXISTS ${PXR_CMAKE_DIR}/lib/python)
        set(PXR_PYTHON_ENABLED TRUE)
    else()
        set(PXR_PYTHON_ENABLED FALSE)
    endif()

    # if(TARGET tf)
    #     get_target_property(TF_COMPILE_DEFINITIONS tf INTERFACE_COMPILE_DEFINITIONS)
    #     foreach(compile_definition ${TF_COMPILE_DEFINITIONS})
    #         string(FIND "${compile_definition}" "PXR_PYTHON_ENABLED=1" result)
    #         if(NOT result EQUAL -1)
    #             set(PXR_PYTHON_ENABLED TRUE)
    #             break()
    #         else()
    #             set(PXR_PYTHON_ENABLED FALSE)
    #         endif()
    #     endforeach()
    # elseif(TARGET usd_ms)
    #     get_target_property(TF_COMPILE_DEFINITIONS usd_ms INTERFACE_COMPILE_DEFINITIONS)
    #     foreach(compile_definition ${TF_COMPILE_DEFINITIONS})
    #         string(FIND "${compile_definition}" "PXR_PYTHON_ENABLED=1" result)
    #         if(NOT result EQUAL -1)
    #             set(PXR_PYTHON_ENABLED TRUE)
    #             break()
    #         else()
    #             set(PXR_PYTHON_ENABLED FALSE)
    #         endif()
    #     endforeach()
    # else()
    #     message(FATAL_ERROR "OpenUSD installation misconfigured - the pxr module was found, but the tf / usd_ms target was not found.")
    # endif()

    message(DEBUG "PXR_PYTHON_ENABLED=${PXR_PYTHON_ENABLED}")

    # if Python is enabled, Python support is required even for compiling the C++ libraries
    if(PXR_PYTHON_ENABLED)
        if(NOT TARGET Python3::Python)
            message(FATAL_ERROR "Python include directories are required but no Python3::Python target could be found!")
        else()
            message(DEBUG "Adding Python targets to ${NAME}")
            target_link_libraries(${NAME} PUBLIC Python3::Python)
        endif()
    endif()

endfunction()

function (set_alias_target_properties ALIAS_NAME)
    #[============================================================[.rst:
    set_alias_target_properties
    ---------------
    Sets properties on the target for which the given ALIAS_NAME
    is an alias target for.  Since alias targets are immutable,
    this allows the caller to set properties on that underlying
    alias target using an alias name.

    Inputs
    ^^^^^^
    * ALIAS_NAME: The name of an alias target that has previously
      been defined.
    * PROPERTIES: A list of items representing property name and
      property value in consecutive list positions.
    #]============================================================]

    set(options )
    set(oneValueArgs )
    set(multiValueArgs
        PROPERTIES)

    cmake_parse_arguments(set_alias_target_properties_args
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN})

    get_target_property(aliased_target ${ALIAS_NAME} ALIASED_TARGET)
    if (NOT aliased_target)
        message(FATAL_ERROR "No underlying target could be found for alias target ${ALAIS_NAME}")
    endif()

    foreach (value in LISTS ${set_alias_target_properties_args_PROPERTIES})

        if (NOT property_key)
            # this item is a property key, hold it
            # and look at the next item
            set(property_key ${value})
        else()
            # we had a key last time
            # so this is the corresponding value
            set_target_properties(${aliased_target}
                PROPERTIES
                    ${property_key}
                    ${value})

            unset(property_key)
        endif()

    endforeach()

    if (property_key)
        message(WARNING "PROPERTIES argument passed key ${property_key} with no value...")
    endif()
    
endfunction()

function (openusd_python_plugin NAME)
    #[============================================================[.rst:
    openusd_python_plugin
    ---------------
    Defines a target to build a shared library representing the
    Python module for the plugin containing the C++ bindings to
    Python.

    Inputs
    ^^^^^^
    * NAME: The name of the plugin, which must be the same name
      passed to build the C++ library plugin.  The python module
      will always have the same name as the C++ library with the
      addition of an `_` character in the beginning.
    * ADDITIONAL_ROOT_DIR (optional): A directory considered as an
      additional root when copying files to preserve relative structure.
      If provided, files included in this directory will install
      to the target module root rather than relative to the module
      root.
    * TARGET_ALIAS (optional): If specified, an alias with the name
      ${TARGET_ALIAS) will be created for the shared library target
      created by this method.
    * MODULE_SUBDIR (optional): If specified, a subdirectory structure
      will be created under the target output directory for the module
      and the compiled python module will be placed in this subdirectory.
    * PYTHON_CPP_FILES: The set of cpp files that will be used to
      compile the Python module.
    * PYTHON_FILES: The set of python files (.py files) that should
      be part of the module and which should be installed with the
      module to the final location.  These files will be installed
      in a way that preserves relative descendent paths where possible.
    
    Outputs
    ^^^^^^^
    * PXR_PLUGIN_PYTHON_${NAME}_MODULE_DIR: Sets this variable to the
      target install directory for the Python module.  Note that this
      is a relative directory (typically camel cased module name) such
      that it can be moved around by setting CMAKE_INSTALL_PREFIX.

    Targets
    ^^^^^^^
    This method will add a shared library target with the name _${NAME}
    with a dependency on the C++ shared library target with the name ${NAME}.

    If a value for TARGET_ALIAS was provided, an alias target with the name
    ${TARGET_ALIAS} will be created.
    
    Note, the module built by this target will have the extension
    .pyd on Windows and .so on Linux, and will also have no prefix
    (e.g. `lib` on Linux) to facilitate loading the Python module
    via the OpenUSD Tf loader.

    The defined target will always link the tf, sdf, and usd targets
    in non-monolithic builds and usd_ms in monolithic builds.  Additional
    target dependencies may be added at higher levels using the `_${NAME}` target.

    #]============================================================]

    set(options )
    set(oneValueArgs
        ADDITIONAL_ROOT_DIR
        TARGET_ALIAS
        MODULE_SUBDIR)
    set(multiValueArgs
        PYTHON_CPP_FILES
        PYTHON_FILES)

    cmake_parse_arguments(openusd_python_plugin_args
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN})

    if (NOT PXR_CMAKE_DIR)
        message(FATAL_ERROR "OpenUSD installation misconfigured - the pxr module was found, but PXR_CMAKE_DIR was not defined.")
    endif()
    
    set(PXR_PLUGIN_PYTHON_TARGET_NAME _${NAME})
    string(SUBSTRING ${NAME} 0 1 namePrefix)
    string(SUBSTRING ${NAME} 1 -1 nameSuffix)
    string(TOUPPER ${namePrefix} namePrefix)
    set(PXR_PLUGIN_PYTHON_MODULE_NAME ${namePrefix}${nameSuffix})

    # create the library and add a dependency on the C++ module
    add_library(${PXR_PLUGIN_PYTHON_TARGET_NAME}
        SHARED
        "${openusd_python_plugin_args_PYTHON_CPP_FILES}")
    add_dependencies(${PXR_PLUGIN_PYTHON_TARGET_NAME}
        ${NAME})

    # add an alias target if desired
    if (openusd_python_plugin_args_TARGET_ALIAS)
        add_library(${openusd_python_plugin_args_TARGET_ALIAS} ALIAS ${PXR_PLUGIN_PYTHON_TARGET_NAME})
    endif()

    add_standard_openusd_options(${PXR_PLUGIN_PYTHON_TARGET_NAME})

    # must be suffixed with pyd on Windows and so on Linux
    # and have no prefix
    set_target_properties(${PXR_PLUGIN_PYTHON_TARGET_NAME}
        PROPERTIES
            PREFIX "")
    if(WIN32)
        set_target_properties(${PXR_PLUGIN_PYTHON_TARGET_NAME}
            PROPERTIES
                SUFFIX ".pyd")
    else()
        set_target_properties(${PXR_PLUGIN_PYTHON_TARGET_NAME}
            PROPERTIES
                SUFFIX ".so")
    endif()

    # compile definitions required for OpenUSD Tf code to load the module
    target_compile_definitions(${PXR_PLUGIN_PYTHON_TARGET_NAME}
        PRIVATE
            MFB_PACKAGE_NAME=${NAME}
            MFB_ALT_PACKAGE_NAME=${NAME}
            MFB_PACKAGE_MODULE=${PXR_PLUGIN_PYTHON_MODULE_NAME})

    # link the required OpenUSD libraries via their targets
    target_link_libraries(${PXR_PLUGIN_PYTHON_TARGET_NAME} PUBLIC tf)
    target_link_libraries(${PXR_PLUGIN_PYTHON_TARGET_NAME} PUBLIC sdf)
    target_link_libraries(${PXR_PLUGIN_PYTHON_TARGET_NAME} PUBLIC usd)

    # python support is required for building python plugins
    target_link_libraries(${PXR_PLUGIN_PYTHON_TARGET_NAME} PUBLIC Python3::Python)

    # although the C++ lib generates a targets file, this isn't available
    # at build time so we link the library explicitly
    target_link_libraries(${PXR_PLUGIN_PYTHON_TARGET_NAME} PUBLIC $<TARGET_LINKER_FILE:${NAME}>)

    if(WIN32)
        if (openusd_python_plugin_args_MODULE_SUBDIR)
            set(CMAKE_INSTALL_BINDIR ${openusd_python_plugin_args_MODULE_SUBDIR})
        else()
            set(CMAKE_INSTALL_BINDIR ${PXR_PLUGIN_PYTHON_MODULE_NAME})
        endif()
        install(TARGETS ${PXR_PLUGIN_PYTHON_TARGET_NAME}
            RUNTIME
                DESTINATION ${CMAKE_INSTALL_BINDIR})
    else()
        if (openusd_python_plugin_args_MODULE_SUBDIR)
            set(CMAKE_INSTALL_LIBDIR ${openusd_python_plugin_args_MODULE_SUBDIR})
        else()
            set(CMAKE_INSTALL_LIBDIR ${PXR_PLUGIN_PYTHON_MODULE_NAME})
        endif()
        install(TARGETS ${PXR_PLUGIN_PYTHON_TARGET_NAME}
            RUNTIME
                DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()

    if(NOT DEFINED PXR_PLUGIN_ROOT)
        get_filename_component(PXR_PLUGIN_ROOT ${CMAKE_PARENT_LIST_FILE} DIRECTORY)
        message(STATUS "-- PXR_PLUGIN_ROOT not assigned, defaulting to: ${PXR_PLUGIN_ROOT}")
    endif()

    # install all python files, but keep relative paths
    if (openusd_python_plugin_args_PYTHON_FILES)
        if (openusd_python_plugin_args_MODULE_SUBDIR)
            set(PXR_PLUGIN_PYTHON_${NAME}_MODULE_DIR ${openusd_python_plugin_args_MODULE_SUBDIR})
        else()
            set(PXR_PLUGIN_PYTHON_${NAME}_MODULE_DIR ${PXR_PLUGIN_PYTHON_MODULE_NAME})
        endif()
        cmake_path(GET CMAKE_PARENT_LIST_FILE PARENT_PATH PXR_PLUGIN_ROOT)
        foreach(python_file ${openusd_python_plugin_args_PYTHON_FILES})
            cmake_path(IS_RELATIVE python_file is_file_relative)
            if (is_file_relative)
                # if it's relative in the plugin source tree, preserve the interior structure
                # by concatenating it with the root target dir
                # otherwise it's relative outside the source tree (e.g. generated __init__.py)
                # and needs to be fully resolved and treated as an absolute path
                # unless it was an additional root, in which case we don't preserve
                # interior structure for this file
                set(absolute_python_file ${PXR_PLUGIN_ROOT}/${python_file})
                cmake_path(NORMAL_PATH absolute_python_file OUTPUT_VARIABLE normalized_python_file)
                string(FIND ${normalized_python_file} ${PXR_PLUGIN_ROOT} root_relative)
                if(NOT root_relative EQUAL -1)
                    # it's relative to the root, so check whether the path includes
                    # is the additional root dir path
                    set(source_python_file_path ${PXR_PLUGIN_ROOT}/${python_file})
                    cmake_path(GET source_python_file_path PARENT_PATH parent_source_python_file_path)
                    cmake_path(GET python_file PARENT_PATH python_file_path)
                    cmake_path(GET python_file FILENAME python_file_name)
                    if (openusd_python_plugin_args_ADDITIONAL_ROOT_DIR AND (parent_source_python_file_path STREQUAL openusd_python_plugin_args_ADDITIONAL_ROOT_DIR))
                        # this one should be absolute rooted
                        install(FILES ${source_python_file_path}
                            DESTINATION ${PXR_PLUGIN_PYTHON_${NAME}_MODULE_DIR}
                            RENAME ${python_file_name})
                    else()
                        install(FILES ${source_python_file_path}
                            DESTINATION ${PXR_PLUGIN_PYTHON_${NAME}_MODULE_DIR}/${python_file_path}
                            RENAME ${python_file_name})
                    endif()
                else()
                    cmake_path(GET python_file FILENAME python_file_name)
                    install(FILES ${python_file}
                        DESTINATION ${PXR_PLUGIN_PYTHON_${NAME}_MODULE_DIR}
                        RENAME ${python_file_name})
                endif()
            else()
                # if it isn't relative, it might either:
                # 1. have been prepended with something like ${CMAKE_CURRENT_LIST_DIR}
                # 2. be an absolute path outside the structure (bad practice, but will handle anyway)
                # in either of these cases, the file will go directly to the module install root
                cmake_path(GET python_file FILENAME python_file_name)
                install(FILES ${python_file}
                    DESTINATION ${PXR_PLUGIN_PYTHON_${NAME}_MODULE_DIR}
                    RENAME ${python_file_name})
            endif()
        endforeach()
    endif()

endfunction()

function (openusd_plugin NAME)
    #[============================================================[.rst:
    openusd_plugin
    ---------------
    Defines a target to build a shared library representing the
    C++ library for an OpenUSD plugin.

    Inputs
    ^^^^^^
    * NAME: The name of the plugin.
    * EXPORT_NAMESPACE (optional): The namespace that will
      be prepended to all targets in the exported targets file.
      If not provided, this will be the lowercase version of the
      target name (NAME).  A trailing "::" will be added to the namespace
      name.
    * TARGET_ALIAS (optional): If provided, an alias target with the
      name ${TARGET_ALIAS} will be created referring to the shared
      library target created by this method.
    * PUBLIC_HEADER_FILES: The set of header files that will be
      used to compile the C++ library.  These headers will also be
      included in the interface definition of the generated cmake
      import target.
    * PRIVATE_HEADER_FILES: Additional header files that will be used
      to compile the C++ library, but will not be included in the
      interface definition fo the generated cmake import target.
    * CPP_FILES: The list of C++ files taht will be used to compile
      the C++ library.
    * RESOURCE_FILES: Additional files that will be included in the
      install targets for the library.  These will be installed by
      default to `plugins/${NAME}/resources` and relative paths
      will be preserved where able.

    Targets
    ^^^^^^^
    This method will add a shared library target with the name ${NAME}.
    The defined target will always link the tf, sdf, and usd targets
    in non-monolithic builds and usd_ms in monolithic builds.  Additional
    target dependencies may be added at higher levels using the `${NAME}` target.

    Alternatively, if TARGET_ALIAS is provided, an alias target with the name
    ${TARGET_ALIAS} will be created.

    This method also defines an export target with the name ${NAME}Targets
    with a directive to install it to the `cmake` target directory (relative
    to CMAKE_INSTALL_PREFIX).

    Finally, this method will configure the `plugInfo.json` file if it is
    includedin the RESOURCE_FILES set with the right paths from the install targets.

    #]============================================================]

    set(options )
    set(oneValueArgs
        EXPORT_NAMESPACE
        TARGET_ALIAS)
    set(multiValueArgs
        PUBLIC_HEADER_FILES
        PRIVATE_HEADER_FILES
        CPP_FILES
        RESOURCE_FILES)

    cmake_parse_arguments(openusd_plugin_args
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN})

    if (NOT PXR_CMAKE_DIR)
        message(FATAL_ERROR "OpenUSD installation misconfigured - the pxr module was found, but PXR_CMAKE_DIR was not defined.")
    endif()

    if (NOT openusd_plugin_args_EXPORT_NAMESPACE)
        string(TOLOWER ${NAME} export_namespace)
        set(export_namespace "${export_namespace}::")
    else()
        set(export_namespace "${openusd_plugin_args_EXPORT_NAMESPACE}::")
    endif()

    add_library(${NAME}
        SHARED
        "${openusd_plugin_args_CPP_FILES}"
        "${openusd_plugin_args_PUBLIC_HEADER_FILES}"
        "${openusd_plugin_args_PRIVATE_HEADER_FILES}"
    )

    if (openusd_plugin_args_TARGET_ALIAS)
        add_library(${openusd_plugin_args_TARGET_ALIAS} ALIAS ${NAME})
    endif()

    add_standard_openusd_options(${NAME})

    string(TOUPPER ${NAME} LIBRARY_NAME_UPPERCASE)
    target_compile_definitions(${NAME} 
        PRIVATE
            ${LIBRARY_NAME_UPPERCASE}_EXPORTS)

    # link with Python if OpenUSD was built with Python enabled
    openusd_link_with_python(${NAME})

    if(TARGET usd_ms)
        target_link_libraries(${NAME} PUBLIC usd_ms)
    else()
        target_link_libraries(${NAME} PUBLIC tf)
        target_link_libraries(${NAME} PUBLIC sdf)
        target_link_libraries(${NAME} PUBLIC usd)
    endif()

    # add the public headers such that they make their way into the
    # correct install location
    set_target_properties(${NAME} PROPERTIES PUBLIC_HEADER "${openusd_plugin_args_PUBLIC_HEADER_FILES}")
    set(CMAKE_TOP_LEVEL_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
    set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/${NAME})

    # install binaries to default directories unless overridden
    # TODO: this won't keep relative header structure, so may need to convert the
    # headers over to install(FILES) but we have to make sure the directory
    # makes it into the export targets
    install(TARGETS ${NAME}
        EXPORT
            ${NAME}Targets
        PUBLIC_HEADER
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        ARCHIVE
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME
            DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES
            DESTINATION ${CMAKE_TOP_LEVEL_INSTALL_INCLUDEDIR})

    # Define the export target for the library
    install(EXPORT ${NAME}Targets
        NAMESPACE ${export_namespace}
        DESTINATION "cmake")

    # install plugin resource files to default location unless overridden
    set(PXR_PLUGIN_RESOURCES_DIRECTORY plugins/${NAME}/resources)
    foreach(resource_file ${openusd_plugin_args_RESOURCE_FILES})
        get_filename_component(file_name ${resource_file} NAME)
        if(${file_name} STREQUAL "plugInfo.json")
            # configure the information in plugInfo.json
            # then install it to the target destination
            if(WIN32)
                cmake_path(RELATIVE_PATH CMAKE_INSTALL_BINDIR
                    BASE_DIRECTORY ${PXR_PLUGIN_RESOURCES_DIRECTORY}
                    OUTPUT_VARIABLE lib_relative_dir)
            else()
                cmake_path(RELATIVE_PATH CMAKE_INSTALL_LIBDIR
                    BASE_DIRECTORY ${PXR_PLUGIN_RESOURCES_DIRECTORY}
                    OUTPUT_VARIABLE lib_relative_dir)
            endif()
            set(PLUG_INFO_ROOT .)
            set(PLUG_INFO_RESOURCE_PATH .)
            set(PLUG_INFO_LIBRARY_PATH ${lib_relative_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}${NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
            if (IS_ABSOLUTE ${resource_file})
                configure_file(${resource_file}
                    ${resource_file}.configured)
            else()
                configure_file(${resource_file}
                    ${CMAKE_CURRENT_SOURCE_DIR}/${resource_file}.configured)
            endif()
            install(FILES ${resource_file}.configured
                DESTINATION ${PXR_PLUGIN_RESOURCES_DIRECTORY}
                RENAME ${file_name})
        else()
            install(FILES ${resource_file}
                DESTINATION ${PXR_PLUGIN_RESOURCES_DIRECTORY})
        endif()
    endforeach()

endfunction()

function (openusd_schema NAME)
    #[============================================================[.rst:
    openusd_schema
    ---------------
    Defines a target to generate OpenUSD code / plugin information
    from a given schema file and:
    * Define a target for the schema's C++ library (if codeful)
    * Define a target for the schema's Python module (if codeful and
      Python is not suppressed).

    This method will use `usdGenSchema` to generate code / plugin files
    and will invoke the openusd_plugin and openusd_python_plugin
    methods, adding the generated files to the list of files defined
    when calling this method.  Refer to those methods for documentation
    about the targets they produce.

    Code / plugin generation is controlled via the GENERATE_SCHEMA option
    (see below).  If your files have been previously generated, you can add those
    files to your file list and leave this option off in which case no generation
    will occur, but the C++ / Python module targets will still be added
    if the schema is codeful.

    If generating code / plugin information for a schema, this method will only
    perform this generation if the provided schema file has changed.  This method
    keeps track of the timestamp of the file via the cmake cache variable
    PXR_${NAME}_SCHEMA_INPUT_TIMESTAMP.

    Note that python is required for schema generation, even if you don't want
    to create the target defining the plugin Python module.  This is because
    the schema generator is a python script and the python executable needs to
    be defined to find it.

    Inputs
    ^^^^^^
    * NAME: The name of the plugin.
    * GENERATE_SCHEMA (optional): Set this to TRUE to invoke code / plugin
      info generation against a schema file.
    * SUPPRESS_PYTHON_MODULE (optional): Set this to TRUE to suppress
      adding the python module target for the schema plugin.
    * SUPPRESS_GENERATE_MODULE_DEPS_CPP (optional): Set this to TRUE to suppress
      automatic generation of the moduleDeps.cpp file, which defines information
      for the tf library to map C++ to Python module when loading plugins.
    * SUPPRESS_GENERATE_WRAP_MODULE_CPP (optional): Set this to TRUE to suppress
      automatic generation of the wrapModule.cpp file, which will define a
      TF_WRAP statement for the module including the TF_WRAP statements for
      all of the classes that need Python bindings for the python module.
    * SUPPRESS_GENERATE_MODULE_INIT_PY (optional): Set this to TRUE to suppress
      automatic generation of an __init__.py file that will be placed at the root
      of the module install directory and will invoke Tf to prepare the python module
      at load time.
    * SCHEMA_FILE: The path to the schema file that will be used by usdGenSchema
      if GENERATE_SCHEMA is TRUE.
    * GENERATE_DIR: A target directory to generate the code / plugin information
      to.  If not provided, this will default to ${CMAKE_CURRENT_BINARY_DIR}/generated.
    * PXR_PLUGIN_DIR (optional): The directory considered the root
      of the plugin source.  If not provided this will default
      to the CMAKE_PARENT_LIST_FILE directory.
    * EXPORT_NAMESPACE (optional): The name of the namespace that will be prepended
      to the export target generated for the C++ plugin.  If nothing is explicitly
      specified, this will default to the lowercase version of NAME.  A trailing "::"
      will be added to the specified name.
    * CPP_TARGET_ALIAS (optional): If provided, an alias target for the C++
      shared library created for codeful schemas will be created.
    * PYTHON_TARGET_ALIAS (optional): If provided, an alias target for the Python
      module created for codeful schemas will be created.
    * MODULE_SUBDIR (optional): If provided, a subdirectory structure will be created
      under the default python module output directory and the python module will
      be placed in this subdirectory.  This option is ignored if SUPPRESS_PYTHON_MODULE
      is set to TRUE.  This value must use / to deliniate directories.
    * PUBLIC_HEADER_FILES: The set of header files that will be
      used to compile the C++ library.  These headers will also be
      included in the interface definition of the generated cmake
      import target.
    * PRIVATE_HEADER_FILES: Additional header files that will be used
      to compile the C++ library, but will not be included in the
      interface definition fo the generated cmake import target.
    * CPP_FILES: The list of C++ files taht will be used to compile
      the C++ library.
    * RESOURCE_FILES: Additional files that will be included in the
      install targets for the library.  These will be installed by
      default to `plugins/${NAME}/resources` and relative paths
      will be preserved where able.
    * PYTHON_CPP_FILES: C++ files that will be included when adding
      the python module target (if python is not suppressed).
    * PYTHON_FILES: .py files that will be included when adding
      the python module target (if python is not suppressed).

    This method requires the following cmake variables to be defined:
    * PXR_OPENUSD_PYTHON_DIR: The directory where the python executable resides

    #]============================================================]

    set (options
        GENERATE_SCHEMA
        SUPPRESS_PYTHON_MODULE
        SUPPRESS_GENERATE_MODULE_DEPS_CPP
        SUPPRESS_GENERATE_WRAP_MODULE_CPP
        SUPPRESS_GENERATE_MODULE_INIT_PY)
    set (oneValueArgs
        SCHEMA_FILE
        GENERATE_DIR
        PXR_PLUGIN_DIR
        EXPORT_NAMESPACE
        CPP_TARGET_ALIAS
        PYTHON_TARGET_ALIAS
        MODULE_SUBDIR)
    set (multiValueArgs
        PUBLIC_HEADER_FILES
        PRIVATE_HEADER_FILES
        CPP_FILES
        RESOURCE_FILES
        PYTHON_CPP_FILES
        PYTHON_FILES)

    cmake_parse_arguments(openusd_schema_args
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN})

    if (openusd_schema_args_MODULE_SUBDIR)
        set(PXR_PYTHON_MODULE_SUBDIR ${openusd_schema_args_MODULE_SUBDIR})
    else()
        set(PXR_PYTHON_MODULE_SUBDIR)
    endif()

    if (openusd_schema_args_GENERATE_SCHEMA)
        if (NOT PXR_CMAKE_DIR)
            message(FATAL_ERROR "OpenUSD installation misconfigured - the pxr module was found, but PXR_CMAKE_DIR was not defined.")
        endif()

        if (NOT PXR_OPENUSD_PYTHON_DIR)
            message(FATAL_ERROR "Schema generation requested but no value for PXR_OPENUSD_PYTHON_DIR was provided.")
        endif()

        if (NOT openusd_schema_args_SCHEMA_FILE)
            message(FATAL_ERROR "Schema generation requested but no value for SCHEMA_FILE was provided.")
        endif()

        if (NOT openusd_schema_args_GENERATE_DIR)
            message(STATUS "No generate directory provided, defaulting to ${CMAKE_CURRENT_BINARY_DIR}/generated")
            set(PXR_GENSCHEMA_GENERATE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
        else()
            set(PXR_GENSCHEMA_GENERATE_DIR ${openusd_schema_args_GENERATE_DIR})
        endif()

        # read the libraryPath from the schema.usda file
        file(STRINGS ${openusd_schema_args_SCHEMA_FILE} SCHEMA_FILE_LINES)
        foreach(line ${SCHEMA_FILE_LINES})
            string(FIND "${line}" "libraryPath" result)
            if (NOT result EQUAL -1)
                # found it, parse out the right hand side of the assignment for the directory
                math(EXPR result "${result} + 12")
                string(SUBSTRING ${line} ${result} -1 line)
                
                # now find the first instance of a "
                string(FIND "${line}" "\"" result)
                if (NOT result EQUAL -1)
                    math(EXPR result "${result} + 1")
                    string(SUBSTRING ${line} ${result} -1 line)

                    # find the right quote
                    string(FIND "${line}" "\"" result)
                    if (NOT result EQUAL -1)
                        string(SUBSTRING ${line} 0 ${result} line)
                        string(COMPARE NOTEQUAL ${line} "." dot_result)
                        string(COMPARE NOTEQUAL ${line} "./" dot_slash_result)
                        if (dot_result AND dot_slash_result)
                            set(SCHEMA_LIBRARY_PATH ${line})
                        endif()
                    endif()
                endif()

                break()
            endif()
        endforeach()

        # users of this method may set PXR_PLUGIN_ROOT to be the
        # directory containing the CMakeLists.txt file for the plugin
        # if this is not set, this method assumes that the calling
        # list file is the one at the root
        if(NOT openusd_schema_args_PXR_PLUGIN_ROOT)
            get_filename_component(PXR_PLUGIN_ROOT ${CMAKE_PARENT_LIST_FILE} DIRECTORY)
            message(STATUS "Plugin root not assigned, defaulting to: ${PXR_PLUGIN_ROOT}")
        endif()

        # cache the schema such that it won't run if the schema input doesn't change
        file(TIMESTAMP ${openusd_schema_args_SCHEMA_FILE} PXR_${NAME}_SCHEMA_INPUT)
        set(PXR_${NAME}_SCHEMA_INPUT_TIMESTAMP ${PXR_${NAME}_SCHEMA_INPUT} CACHE STRING "Timestamp of input to schema generator" FORCE)
        if (NOT PXR_${NAME}_SCHEMA_INPUT_TIMESTAMP STREQUAL PXR_${NAME}_SCHEMA_INPUT_INTERNAL_TIMESTAMP)
            set(PXR_OPENUSD_PYTHON_EXE ${PXR_OPENUSD_PYTHON_DIR}/python${CMAKE_EXECUTABLE_SUFFIX})
            set(PXR_GENSCHEMA_VENV_PATH ${CMAKE_CURRENT_BINARY_DIR}/_schemagen_venv)
            if (WIN32)
                set(PXR_GENSCHEMA_PYTHON_EXE ${PXR_GENSCHEMA_VENV_PATH}/Scripts/python${CMAKE_EXECUTABLE_SUFFIX})
                set(PXR_GENSCHEMA_PIP_EXE ${PXR_GENSCHEMA_VENV_PATH}/Scripts/pip${CMAKE_EXECUTABLE_SUFFIX})
            else()
                set(PXR_GENSCHEMA_PYTHON_EXE ${PXR_GENSCHEMA_VENV_PATH}/bin/python${CMAKE_EXECUTABLE_SUFFIX})
                set(PXR_GENSCHEMA_PIP_EXE ${PXR_GENSCHEMA_VENV_PATH}/bin/pip${CMAKE_EXECUTABLE_SUFFIX})
            endif()
            set(PXR_GENSCHEMA_RUN_SCRIPT ${USD_PLUGIN_CMAKE_UTILS_ROOT}/scripts/genSchema.py)
            set(PXR_USDGENSCHEMA_SCRIPT ${PXR_CMAKE_DIR}/bin/usdGenSchema)

            # create the virtual directory
            execute_process(
                COMMAND ${PXR_OPENUSD_PYTHON_EXE} -m venv ${PXR_GENSCHEMA_VENV_PATH}
                RESULT_VARIABLE result
            )
            if(NOT result EQUAL 0)
                message(FATAL_ERROR "Creation of virtual environment for schema generation failed!")
            endif()

            # pip install jinja (required by usdGenSchema)
            # TODO: may need to create a requirements.txt file and pip install from that
            execute_process(
                COMMAND ${PXR_GENSCHEMA_PIP_EXE} install jinja2==3.1.4
                RESULT_VARIABLE result
            )
            if(NOT result EQUAL 0)
                message(FATAL_ERROR "Installing jinja templates for schema generation failed!")
            endif()

            # run the schema generation process and create a file we can include
            # such that the generated artifacts get included in the target
            if (TBB_ROOT)
                set(ENV{TBB_ROOT} ${TBB_ROOT})
            endif()
            if (BOOST_ROOT)
                set(ENV{BOOST_ROOT} ${BOOST_ROOT})
            endif()
            execute_process(
                COMMAND ${PXR_GENSCHEMA_PYTHON_EXE} ${PXR_GENSCHEMA_RUN_SCRIPT} ${openusd_schema_args_SCHEMA_FILE} --generate-dir=${PXR_GENSCHEMA_GENERATE_DIR} --usd-root=${PXR_CMAKE_DIR} --python-root=${PXR_OPENUSD_PYTHON_DIR}
                RESULT_VARIABLE result
            )
            if(NOT result EQUAL 0)
                message(FATAL_ERROR "Schema generation failed!")
            endif()

            include(${PXR_GENSCHEMA_GENERATE_DIR}/gen_schema_output.cmake)
            if (SCHEMA_LIBRARY_PATH)
                make_directory(${PXR_GENSCHEMA_GENERATE_DIR}/${SCHEMA_LIBRARY_PATH})
                foreach(header_file ${PXR_GENERATED_PUBLIC_HEADERS})
                    file(RENAME ${PXR_GENSCHEMA_GENERATE_DIR}/${header_file} ${PXR_GENSCHEMA_GENERATE_DIR}/${SCHEMA_LIBRARY_PATH}/${header_file})
                endforeach()
            endif()

            # remove the virtual environment
            file(REMOVE_RECURSE ${PXR_GENSCHEMA_VENV_PATH})

            # set the cache variable so that it doesn't run again
            set(PXR_${NAME}_SCHEMA_INPUT_INTERNAL_TIMESTAMP ${PXR_${NAME}_SCHEMA_INPUT_TIMESTAMP} CACHE INTERNAL "for internal use only; do not modify")
        else()
            # include the generated output variables and compose them with what was passed in
            include(${PXR_GENSCHEMA_GENERATE_DIR}/gen_schema_output.cmake)
        endif()

        if (SCHEMA_LIBRARY_PATH)
            foreach(header_file ${PXR_GENERATED_PUBLIC_HEADERS})
                list(APPEND PXR_GENERATED_PUBLIC_HEADERS_SOURCE ${SCHEMA_LIBRARY_PATH}/${header_file})
            endforeach()
        else()
            set(PXR_GENERATED_PUBLIC_HEADERS_SOURCE ${PXR_GENERATED_PUBLIC_HEADERS})
        endif()

        list(TRANSFORM PXR_GENERATED_PUBLIC_HEADERS_SOURCE PREPEND ${PXR_GENSCHEMA_GENERATE_DIR}/)
        list(TRANSFORM PXR_GENERATED_CPP_FILES PREPEND ${PXR_GENSCHEMA_GENERATE_DIR}/)
        list(TRANSFORM PXR_GENERATED_RESOURCE_FILES PREPEND ${PXR_GENSCHEMA_GENERATE_DIR}/)
        list(TRANSFORM PXR_GENERATED_PYTHON_CPP_FILES PREPEND ${PXR_GENSCHEMA_GENERATE_DIR}/)
        list(TRANSFORM PXR_GENERATED_PYTHON_FILES PREPEND ${PXR_GENSCHEMA_GENERATE_DIR}/)
        set(PXR_PLUGIN_PRIVATE_HEADER_FILES ${openusd_schema_args_PRIVATE_HEADER_FILES})
        foreach (file ${PXR_GENERATED_PUBLIC_HEADERS_SOURCE})
            cmake_path(RELATIVE_PATH file BASE_DIRECTORY ${PXR_PLUGIN_ROOT} OUTPUT_VARIABLE file_relative)
            list(APPEND PXR_GENERATED_RELATIVE_PUBLIC_HEADERS ${file_relative})
        endforeach()
        foreach (file ${PXR_GENERATED_CPP_FILES})
            cmake_path(RELATIVE_PATH file BASE_DIRECTORY ${PXR_PLUGIN_ROOT} OUTPUT_VARIABLE file_relative)
            list(APPEND PXR_GENERATED_RELATIVE_CPP_FILES ${file_relative})
        endforeach()
        foreach (file ${PXR_GENERATED_RESOURCE_FILES})
            cmake_path(RELATIVE_PATH file BASE_DIRECTORY ${PXR_PLUGIN_ROOT} OUTPUT_VARIABLE file_relative)
            list(APPEND PXR_GENERATED_RELATIVE_RESOURCE_FILES ${file_relative})
        endforeach()
        foreach (file ${PXR_GENERATED_PYTHON_CPP_FILES})
            cmake_path(RELATIVE_PATH file BASE_DIRECTORY ${PXR_PLUGIN_ROOT} OUTPUT_VARIABLE file_relative)
            list(APPEND PXR_GENERATED_RELATIVE_PYTHON_CPP_FILES ${file_relative})
        endforeach()
        foreach (file ${PXR_GENERATED_PYTHON_FILES})
            cmake_path(RELATIVE_PATH file BASE_DIRECTORY ${PXR_PLUGIN_ROOT} OUTPUT_VARIABLE file_relative)
            list(APPEND PXR_GENERATED_RELATIVE_PYTHON_FILES ${file_relative})
        endforeach()

        # configure the moduleDeps.cpp file
        if((NOT ${PXR_SCHEMA_IS_CODELESS}) AND (NOT ${openusd_schema_args_SUPPRESS_GENERATE_MODULE_DEPS_CPP}))
            string(SUBSTRING ${NAME} 0 1 namePrefix)
            string(SUBSTRING ${NAME} 1 -1 nameSuffix)
            string(TOUPPER ${namePrefix} namePrefix)
            set(PXR_PLUGIN_NAME ${NAME})
            if (PXR_PYTHON_MODULE_SUBDIR)
                # convert subdir to dot structure
                string(REPLACE "/" "." module_dot_structure ${PXR_PYTHON_MODULE_SUBDIR})
                set(PXR_PLUGIN_PYTHON_MODULE_NAME ${module_dot_structure})
            else()
                set(PXR_PLUGIN_PYTHON_MODULE_NAME ${namePrefix}${nameSuffix})
            endif()
            set(module_deps_template ${USD_PLUGIN_CMAKE_UTILS_ROOT}/templates/moduleDeps.cpp.in)
            set(module_deps_configured ${PXR_GENSCHEMA_GENERATE_DIR}/moduleDeps.cpp)
            configure_file(${module_deps_template}
                ${module_deps_configured})
            cmake_path(RELATIVE_PATH module_deps_configured BASE_DIRECTORY ${PXR_PLUGIN_ROOT} OUTPUT_VARIABLE file_relative)
            list(APPEND PXR_GENERATED_RELATIVE_CPP_FILES ${file_relative})
        endif()

        # configure the wrapModule.cpp file
        if((NOT ${PXR_SCHEMA_IS_CODELESS}) AND (NOT ${openusd_schema_args_SUPPRESS_GENERATE_WRAP_MODULE_CPP}) AND (NOT ${openusd_schema_args_SUPPRESS_PYTHON_MODULE}))
            set(wrap_module_template ${USD_PLUGIN_CMAKE_UTILS_ROOT}/templates/wrapModule.cpp.in)
            set(wrap_module_configured ${PXR_GENSCHEMA_GENERATE_DIR}/wrapModule.cpp)
            configure_file(${wrap_module_template}
                ${wrap_module_configured})
            cmake_path(RELATIVE_PATH wrap_module_configured BASE_DIRECTORY ${PXR_PLUGIN_ROOT} OUTPUT_VARIABLE file_relative)
            list(APPEND PXR_GENERATED_RELATIVE_PYTHON_CPP_FILES ${file_relative})
        endif()

        # configure the __init__.py file
        if((NOT ${PXR_SCHEMA_IS_CODELESS}) AND (NOT ${openusd_schema_args_SUPPRESS_GENERATE_MODULE_INIT_PY}) AND (NOT ${openusd_schema_args_SUPPRESS_PYTHON_MODULE}))
            set(PXR_PLUGIN_PYTHON_TARGET_NAME _${NAME})
            set(module_init_template ${USD_PLUGIN_CMAKE_UTILS_ROOT}/templates/__init__.py.in)
            set(module_init_configured ${PXR_GENSCHEMA_GENERATE_DIR}/__init__.py)
            configure_file(${module_init_template}
                ${module_init_configured})
            cmake_path(RELATIVE_PATH module_init_configured BASE_DIRECTORY ${PXR_PLUGIN_ROOT} OUTPUT_VARIABLE file_relative)
            list(APPEND PXR_GENERATED_RELATIVE_PYTHON_FILES ${file_relative})
        endif()

        set(PXR_PLUGIN_PUBLIC_HEADER_FILES ${openusd_schema_args_PUBLIC_HEADER_FILES} ${PXR_GENERATED_RELATIVE_PUBLIC_HEADERS})
        set(PXR_PLUGIN_CPP_FILES ${openusd_schema_args_CPP_FILES} ${PXR_GENERATED_RELATIVE_CPP_FILES})
        set(PXR_PLUGIN_RESOURCE_FILES ${openusd_schema_args_RESOURCE_FILES} ${PXR_GENERATED_RELATIVE_RESOURCE_FILES})
        set(PXR_PLUGIN_PYTHON_CPP_FILES ${openusd_schema_args_PYTHON_CPP_FILES} ${PXR_GENERATED_RELATIVE_PYTHON_CPP_FILES})
        set(PXR_PLUGIN_PYTHON_FILES ${openusd_schema_args_PYTHON_FILES} ${PXR_GENERATED_RELATIVE_PYTHON_FILES})
    else()
        set(PXR_PLUGIN_PUBLIC_HEADER_FILES ${openusd_schema_args_PUBLIC_HEADER_FILES})
        set(PXR_PLUGIN_PRIVATE_HEADER_FILES ${openusd_schema_args_PRIVATE_HEADER_FILES})
        set(PXR_PLUGIN_CPP_FILES ${openusd_schema_args_CPP_FILES})
        set(PXR_PLUGIN_RESOURCE_FILES ${openusd_schema_args_RESOURCE_FILES})
        set(PXR_PLUGIN_PYTHON_CPP_FILES ${openusd_schema_args_PYTHON_CPP_FILES})
        set(PXR_PLUGIN_PYTHON_FILES ${openusd_schema_args_PYTHON_FILES})
    endif()

    # add the schema to the set of resource files
    list(APPEND PXR_PLUGIN_RESOURCE_FILES ${openusd_schema_args_SCHEMA_FILE})

    # build the schema as a plugin now that the schema generation is complete
    if(NOT ${PXR_SCHEMA_IS_CODELESS})
        openusd_plugin(${NAME}
            EXPORT_NAMESPACE ${openusd_schema_args_EXPORT_NAMESPACE}
            PUBLIC_HEADER_FILES ${PXR_PLUGIN_PUBLIC_HEADER_FILES}
            PRIVATE_HEADER_FILES ${PXR_PLUGIN_PRIVATE_HEADER_FILES}
            CPP_FILES ${PXR_PLUGIN_CPP_FILES}
            RESOURCE_FILES ${PXR_PLUGIN_RESOURCE_FILES}
            TARGET_ALIAS ${openusd_schema_args_CPP_TARGET_ALIAS})

        target_include_directories(${NAME}
            PRIVATE
            ${PXR_GENSCHEMA_GENERATE_DIR})
        set(${NAME}_IncludeDirs ${PXR_GENSCHEMA_GENERATE_DIR} CACHE INTERNAL "") 

        # build the python plugin by default, unless the caller wants to suppress its creation
        if(NOT ${openusd_schema_args_SUPPRESS_PYTHON_MODULE})
            openusd_python_plugin(${NAME}
                ADDITIONAL_ROOT_DIR ${PXR_GENSCHEMA_GENERATE_DIR}
                PYTHON_CPP_FILES ${PXR_PLUGIN_PYTHON_CPP_FILES}
                PYTHON_FILES ${PXR_PLUGIN_PYTHON_FILES}
                TARGET_ALIAS ${openusd_schema_args_PYTHON_TARGET_ALIAS}
                MODULE_SUBDIR ${PXR_PYTHON_MODULE_SUBDIR})

            target_include_directories(_${NAME}
                PRIVATE
                ${PXR_GENSCHEMA_GENERATE_DIR})
        endif()
    else()
        set(PXR_PLUGIN_RESOURCES_DIRECTORY plugins/${NAME}/resources)
        foreach(resource_file ${PXR_PLUGIN_RESOURCE_FILES})
            get_filename_component(file_name ${resource_file} NAME)
            if(${file_name} STREQUAL "plugInfo.json")
                # configure the information in plugInfo.json
                # then install it to the target destination
                set(PLUG_INFO_ROOT .)
                set(PLUG_INFO_RESOURCE_PATH .)
                set(PLUG_INFO_LIBRARY_PATH "")
                if (IS_ABSOLUTE ${resource_file})
                    configure_file(${resource_file}
                        ${resource_file}.configured)
                else()
                    configure_file(${resource_file}
                        ${CMAKE_CURRENT_SOURCE_DIR}/${resource_file}.configured)
                endif()
                install(FILES ${resource_file}.configured
                    DESTINATION ${PXR_PLUGIN_RESOURCES_DIRECTORY}
                    RENAME ${file_name})
            else()
                install(FILES ${resource_file}
                    DESTINATION ${PXR_PLUGIN_RESOURCES_DIRECTORY})
            endif()
        endforeach()
    endif()

endfunction()
