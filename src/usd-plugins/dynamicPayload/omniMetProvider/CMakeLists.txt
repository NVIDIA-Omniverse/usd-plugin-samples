# SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
include(OmniMetProviderDependencies.cmake)

set(omniMetProvider_headers
    ${CMAKE_CURRENT_LIST_DIR}/api.h
    ${CMAKE_CURRENT_LIST_DIR}/omniMetProvider.h)

set(omniMetProvider_sources
    ${CMAKE_CURRENT_LIST_DIR}/omniMetProvider.cpp)

set(omniMetProvider_resources
    ${CMAKE_CURRENT_LIST_DIR}/plugInfo.json)

# calls openusd_plugin build helper from NvPxrPlugin
openusd_plugin(omniMetProvider
    PRIVATE_HEADER_FILES ${omniMetProvider_headers}
    CPP_FILES ${omniMetProvider_sources}
    RESOURCE_FILES ${omniMetProvider_resources})

# setup dependency to edfFileFormatProvider
target_include_directories(omniMetProvider
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../../fileFormat/edfFileFormat)
target_link_libraries(omniMetProvider
    PRIVATE
        edfFileFormat)

# setup external dependencies
target_link_libraries(omniMetProvider
    PRIVATE
        libcurl::libcurl
        zlib::zlib)
