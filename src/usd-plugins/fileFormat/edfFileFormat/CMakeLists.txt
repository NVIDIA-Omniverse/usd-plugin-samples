# SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#

set(edfFileFormat_public_headers
    ${CMAKE_CURRENT_LIST_DIR}/api.h
    ${CMAKE_CURRENT_LIST_DIR}/iEdfDataProvider.h
    ${CMAKE_CURRENT_LIST_DIR}/edfDataProviderFactory.h)

set(edfFileFormat_private_headers
    ${CMAKE_CURRENT_LIST_DIR}/edfData.h
    ${CMAKE_CURRENT_LIST_DIR}/edfPluginManager.h
    ${CMAKE_CURRENT_LIST_DIR}/edfFileFormat.h)

set(edfFileFormat_sources
    ${CMAKE_CURRENT_LIST_DIR}/edfData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/edfDataProviderFactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/edfPluginManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/edfFileFormat.cpp
    ${CMAKE_CURRENT_LIST_DIR}/iEdfDataProvider.cpp)

set(edfFileFormat_resources
    ${CMAKE_CURRENT_LIST_DIR}/plugInfo.json)

# calls openusd_plugin build helper from NvPxrPlugin
openusd_plugin(edfFileFormat
    PUBLIC_HEADER_FILES ${edfFileFormat_public_headers}
    PRIVATE_HEADER_FILES ${edfFileFormat_private_headers}
    CPP_FILES ${edfFileFormat_sources}
    RESOURCE_FILES ${edfFileFormat_resources})
