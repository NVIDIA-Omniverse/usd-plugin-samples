# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
cmake_minimum_required(VERSION 3.23.1)

project(usd-plugins CXX)

# include our packman dependencies
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(PackmanDeps)

# include NVIDIA Pixar Plugin build tools that simplify building OpenUSD plugins
include(NvPxrPlugin)

# set target directories
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/_install")
set(CMAKE_INSTALL_BINDIR "bin")
set(CMAKE_INSTALL_LIBDIR "lib")
set(CMAKE_INSTALL_INCLUDEDIR "include")

add_subdirectory(src/usd-plugins/schema/omniExampleCodelessSchema)
add_subdirectory(src/usd-plugins/schema/omniExampleSchema)
add_subdirectory(src/usd-plugins/schema/omniMetSchema)