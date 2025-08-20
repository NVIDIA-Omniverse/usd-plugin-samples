# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#

# companion to TBBConfig.cmake
# determines the version being used by the OpenUSD package pulled
# NVIDIA prebuilt packages vendor TBB, so if the user didn't specify
# their own TBB (by setting TBB_ROOT) the vendored TBB will be used
set(TBB_STDDEF_H_FILE ${PXR_CMAKE_DIR}/include/tbb/tbb_stddef.h)
if (EXISTS ${TBB_STDDEF_H_FILE})
    # read the version information from the vendored TBB header
    file(STRINGS ${TBB_STDDEF_H_FILE} TBB_STDDEF_H_FILE_LINES)
    foreach(line ${TBB_STDDEF_H_FILE_LINES})
        string(FIND "${line}" "TBB_VERSION_MAJOR" result)
        if (NOT result EQUAL -1)
            # found major version
            math(EXPR result "${result} + 18")
            string(SUBSTRING ${line} ${result} -1 line)
            set(TBB_VERSION_MAJOR ${line})
            continue()
        endif()
        string(FIND "${line}" "TBB_VERSION_MINOR" result)
        if(NOT result EQUAL -1)
            # found minor version
            math(EXPR result "${result} + 18")
            string(SUBSTRING ${line} ${result} -1 line)
            set(TBB_VERSION_MINOR ${line})
            break()
        endif()
    endforeach()

    if (TBB_VERSION_MAJOR)
        set(TBB_VERSION ${TBB_VERSION_MAJOR}.${TBB_VERSION_MINOR})
        set(TBB_VERSION_COUNT 2)
        set(PACKAGE_VERSION ${TBB_VERSION})
        if (PACKAGE_FIND_VERSION)
            if (${PACKAGE_FIND_VERSION} EQUAL ${TBB_VERSION})
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
