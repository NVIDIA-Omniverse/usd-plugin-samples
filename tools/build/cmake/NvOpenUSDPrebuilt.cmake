# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#

#[===============================================================[.rst:
NvOpenUSDPrebuilt
-----------

Provides targets for depending on NVIDIA's pre-built OpenUSD
plugins.  When this dependency option is ON and a valid selected
version of OpenUSD is configured, these targets will download
NVIDIA's prebuilt OpenUSD binary for that version and set add
the deployed path to CMAKE_PREFIX_PATH such that find_package(pxr)
will find pxrConfig.cmake from the prebuilt binaries.

This module is currently dependent on NVIDIA's packman tool to be
present in the tools/packman directory of a repository.

Options
^^^^^^^

This module provides the following options to build with NVIDIA prebuilt
OpenUSD binaries:

``NV_USE_PREBUILT_OPENUSD_BINARIES``
  ON (default) to use NVIDIA prebuilt OpenUSD binaries.

``NV_OPENUSD_BINARY_VERSION``
  A string denoting the version of OpenUSD to build against.

#]===============================================================]

# option to use NVIDIA's prebuilt binaries
# these options can be turned off to provide your own OpenUSD and Python build
option (NV_USE_PREBUILT_OPENUSD_BINARIES "Use NVIDIA's prebuilt OpenUSD binaries" ON)
option (NV_REQUIRE_DEBUG_LIBRARIES "Whether to use the debug or release versions of the prebuilt OpenUSD binaries" OFF)
set (NV_OPENUSD_BINARY_VERSION "22.11" CACHE STRING "Version of OpenUSD to retrieve a prebuilt binary for")
set_property (CACHE NV_OPENUSD_BINARY_VERSION PROPERTY STRINGS "22.11;24.05;25.02;25.08")
set (NV_PACKMAN_PATH "${CMAKE_CURRENT_LIST_DIR}/../../packman" CACHE STRING "Path to NVIDIA's packman tool containing the packman or packman.cmd file")

# package versions for NVIDIA prebuilt binaries
set (NV_OPENUSD_2211_VER "22.11.nv.0.2.7809.a75d767f")
set (NV_OPENUSD_2405_VER "")
set (NV_OPENUSD_2502_VER "")
set (NV_OPENUSD_2508_VER "")
set (NV_PYTHON_310_VER "3.10.18+nv1")
set (NV_PYTHON_311_VER "3.11.13+nv1")
set (NV_PYTHON_312_VER "3.12.11+nv1")

# configuration check - NVIDIA only provides certain OpenUSD versions compatible with certain Python versions
# based on the values entered, packman package names are formed and passed to packman to download
if (NV_USE_PREBUILT_OPENUSD_BINARIES)
    # figure out which platform we are on
    if (WIN32)
        if (NV_OPENUSD_BINARY_VERSION STREQUAL "22.11")
            set (NV_PLATFORM_ARCH "win64")
        else()
            set (NV_PLATFORM_ARCH "windows-x86_64")
        endif()
        set (NV_LEGACY_PLATFORM_ARCH "windows-x86_64")
    else()
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
            if (NV_OPENUSD_BINARY_VERSION STREQUAL "22.11")
                set (NV_PLATFORM_ARCH "linux-aarch64")
            else()
                set (NV_PLATFORM_ARCH "manylinux_2_35_aarch64")
            endif()
            set (NV_LEGACY_PLATFORM_ARCH "linux-aarch64")
        else()
            if (NV_OPENUSD_BINARY_VERSION STREQUAL "22.11")
                set (NV_PLATFORM_ARCH "linux64")
            else()
                set (NV_PLATFORM_ARCH "manylinux_2_35_x86_64")
            endif()
            set (NV_LEGACY_PLATFORM_ARCH "linux-x86_64")
        endif()
    endif()

    # figure out which config we are running
    if (NV_REQUIRE_DEBUG_LIBRARIES)
        set (NV_CONFIG "debug")
    else()
        set (NV_CONFIG "release")
    endif()

    # set the right Python package name and verison that pairs with the selected OpenUSD version
    set (NV_PYTHON_PACKAGE_NAME "python")
    if (NV_OPENUSD_BINARY_VERSION STREQUAL "22.11")
        set (NV_PYTHON_BINARY_VERSION "310")
        set (NV_PYTHON_PACKAGE_VERSION ${NV_PYTHON_310_VER}-${NV_LEGACY_PLATFORM_ARCH})
        set (NV_USD ON)
        set (NV_LEGACY_LINUX "")
        if (NV_PLATFORM_ARCH STREQUAL "linux64")
            set (NV_LEGACY_LINUX "-centos")
        endif()
        set (NV_OPENUSD_PACKAGE_VERSION ${NV_OPENUSD_2211_VER}-${NV_PLATFORM_ARCH}_py${NV_PYTHON_BINARY_VERSION}${NV_LEGACY_LINUX}_${NV_CONFIG}-dev_omniverse)
    elseif (NV_OPENUSD_BINARY_VERSION STREQUAL "24.05")
        set (NV_PYTHON_BINARY_VERSION "311")
        set (NV_PYTHON_PACKAGE_VERSION ${NV_PYTHON_311_VER}-${NV_LEGACY_PLATFORM_ARCH})
        set (NV_OPENUSD_PACKAGE_VERSION ${NV_OPENUSD_2405_VER})
    else()
        set (NV_PYTHON_BINARY_VERSION "312")
        set (NV_PYTHON_PACKAGE_VERSION ${NV_PYTHON_312_VER}-${NV_LEGACY_PLATFORM_ARCH})
        if (NV_OPENUSD_BINARY_VERSION STREQUAL "25.02")
            set (NV_OPENUSD_PACKAGE_VERSION ${NV_OPENUSD_2502_VER})
        elseif (NV_OPENUSD_BINARY_VERSION STREQUAL "25.08")
            set (NV_OPENUSD_PACKAGE_VERSION ${NV_OPENUSD_2508_VER})
        else()
            message(FATAL_ERROR "Selected NVIDIA OpenUSD Version not supported! (${NV_OPENUSD_PACKAGE_VERSION})")
        endif()
    endif()

    # set the OpenUSD package name and version
    if (NV_OPENUSD_BINARY_VERSION STREQUAL "22.11")
        set (NV_OPENUSD_PACKAGE_NAME "nv-usd")
    else()
        set (NV_OPENUSD_PACKAGE_NAME "usd.py${NV_PYTHON_BINARY_VERSION}.${NV_PLATFORM_ARCH}.stock.${NV_CONFIG}")
    endif()
    
    # we have the OpenUSD and Python package names and versions
    # need to ask packman to download them
    if (NOT EXISTS ${NV_PACKMAN_PATH})
        message(FATAL_ERROR "Provided packman path (${NV_PACKMAN_PATH}) does not exist!")
    endif()

    if (WIN32)
        set (NV_PACKMAN_EXE ${NV_PACKMAN_PATH}/packman.cmd)
    else()
        set (NV_PACKMAN_EXE ${NV_PACKMAN_PATH}/packman)
    endif()

    # install and link the OpenUSD package
    execute_process(COMMAND
        ${NV_PACKMAN_EXE}
        "install"
        "-q"
        "-l"
        "_build/usd-deps/usd"
        ${NV_OPENUSD_PACKAGE_NAME}
        ${NV_OPENUSD_PACKAGE_VERSION}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../..
        COMMAND_ERROR_IS_FATAL ANY)

    # install and link the Python package
    execute_process(COMMAND
        ${NV_PACKMAN_EXE}
        "install"
        "-q"
        "-l"
        "_build/usd-deps/python"
        ${NV_PYTHON_PACKAGE_NAME}
        ${NV_PYTHON_PACKAGE_VERSION}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../..
        COMMAND_ERROR_IS_FATAL ANY)

    # set PXR_OPENUSD_PYTHON_DIR to the directory we just pulled python to
    set (PXR_OPENUSD_PYTHON_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../_build/usd-deps/python)

    # also tell cmake where to find the xConfig files it's going to look for when we do
    # an e.g., find_package(pxr)
    # this includes:
    #  - the prebuilt OpenUSD libraries
    #  - the vendored xConfig files in that prebuilt package (e.g., MaterialX)
    #  - the xConfig files supplied here to set up the other vendored dependencies (e.g., TBB, OpenSubdiv, Imath)
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../_build/usd-deps/usd)
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../_build/usd-deps/usd/lib/cmake)
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR})

    # if we are working with 22.11, pxrConfig.config doesn't have a find_package(TBB) anywhere
    # so we have to make sure we pick that up from our own configs
    # but our configs rely on PXR_CMAKE_DIR being set, so set that up too
    if (NV_OPENUSD_BINARY_VERSION STREQUAL "22.11")
        set (PXR_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../_build/usd-deps/usd)
        find_package(TBB CONFIG)
    endif()
endif()
