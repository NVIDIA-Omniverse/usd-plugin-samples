# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#

# companion to OpenSubdivConfig.cmake
# determines the version being used by the OpenUSD package pulled
# NVIDIA prebuilt packages vendor TBB, so if the user didn't specify
# their own OpenSubdiv (by setting OpenSubdiv_DIR) the vendored OpenSubdiv will be used
set(OpenSubdiv_VERSION_H_FILE ${PXR_CMAKE_DIR}/include/opensubdiv/version.h)
if (EXISTS ${OpenSubdiv_VERSION_H_FILE})
    # read the version information from the vendored OpenSubdiv header
    file(STRINGS ${OpenSubdiv_VERSION_H_FILE} OpenSubdiv_VERSION_H_FILE_LINES)
    foreach(line ${OpenSubdiv_VERSION_H_FILE_LINES})
        string(FIND "${line}" "OPENSUBDIV_VERSION " result)
        if (NOT result EQUAL -1)
            # found version string
            math(EXPR result "${result} + 19")
            string(SUBSTRING ${line} ${result} -1 line)
            set(OpenSubDiv_VERSION ${line})
            continue()
        endif()
        string(FIND "${line}" "OPENSUBDIV_VERSION_MAJOR" result)
        if (NOT result EQUAL -1)
            # found major version
            math(EXPR result "${result} + 25")
            string(SUBSTRING ${line} ${result} -1 line)
            set(OpenSubdiv_VERSION_MAJOR ${line})
            continue()
        endif()
        string(FIND "${line}" "OPENSUBDIV_VERSION_MINOR" result)
        if(NOT result EQUAL -1)
            # found minor version
            math(EXPR result "${result} + 25")
            string(SUBSTRING ${line} ${result} -1 line)
            set(OpenSubdiv_VERSION_MINOR ${line})
            continue()
        endif()
        string(FIND "${line}" "OPENSUBDIV_VERSION_PATCH" result)
        if (NOT result EQUAL -1)
            # found patch version
            math(EXPR result "${result} + 25")
            string(SUBSTRING ${line} ${result} -1 line)
            set(OpenSubdiv_VERSION_PATCH ${line})
            break()
        endif()
    endforeach()

    if (OpenSubdiv_VERSION_MAJOR)
        set(OpenSubdiv_VERSION_COUNT 3)
        set(PACKAGE_VERSION ${OpenSubdiv_VERSION})
        if (PACKAGE_FIND_VERSION)
            if (${PACKAGE_FIND_VERSION} EQUAL ${OpenSubdiv_VERSION_MAJOR}.${OpenSubdiv_VERSION_MINOR}.${OpenSubdiv_VERSION_PATCH})
                set(PACKAGE_VERSION_EXACT 1)
            else()
                # assume compatibility since it's what the build
                # was built with
                set(PACKAGE_VERSION_COMPATIBLE 1)
            endif()
        else()
            # no specific version requested, so we can only assume
            # it's compatible
            set(PACKAGE_VERSION_COMPATIBLE 1)
        endif()
    else()
        # we didn't find a version
        set(PACKAGE_VERSION_UNSUITABLE 1)
    endif()
else()
    set(PACKAGE_VERSION_UNSUITABLE 1)
endif()
