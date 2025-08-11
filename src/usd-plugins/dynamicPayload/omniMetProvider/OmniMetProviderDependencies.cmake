# SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
# package versions prebuild dependencies
set (LIBCURL_PACKAGE_NAME "libcurl")
set (LIBCURL_VER "8.1.2-3")
set (ZLIB_PACKAGE_NAME "zlib")
set (ZLIB_VER "1.2.13+nv1")
set (OPENSSL_PACKAGE_NAME "openssl")
set (OPENSSL_VER "3.0.10-3")

# get full package name
if (WIN32)
    set (LIBCURL_PACKAGE_VERSION ${LIBCURL_VER}-windows-x86_64-static-release)
    set (ZLIB_PACKAGE_VERSION ${ZLIB_VER}-windows-x86_64)
else()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
        set (LIBCURL_PACKAGE_VERSION ${LIBCURL_VER}-linux-aarch64-static-release)
        set (ZLIB_PACKAGE_VERSION ${ZLIB_VER}-linux-aarch64)
        set (OPENSSL_PACKAGE_VERSION ${OPENSSL_VER}-linux-aarch64-static-release)
    else()
        set (LIBCURL_PACKAGE_VERSION ${LIBCURL_VER}-linux-x86_64-static-release)
        set (ZLIB_PACKAGE_VERSION ${ZLIB_VER}-linux-x86_64)
        set (OPENSSL_PACKAGE_VERSION ${OPENSSL_VER}-linux-x86_64-static-release)
    endif()
endif()

# install and link the dependency package
execute_process(COMMAND
    ${NV_PACKMAN_EXE}
    "install"
    "-q"
    "-l"
    "_build/target-deps/libcurl"
    ${LIBCURL_PACKAGE_NAME}
    ${LIBCURL_PACKAGE_VERSION}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../../..
    COMMAND_ERROR_IS_FATAL ANY)

execute_process(COMMAND
    ${NV_PACKMAN_EXE}
    "install"
    "-q"
    "-l"
    "_build/target-deps/zlib"
    ${ZLIB_PACKAGE_NAME}
    ${ZLIB_PACKAGE_VERSION}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../../..
    COMMAND_ERROR_IS_FATAL ANY)

if (NOT WIN32)
    execute_process(COMMAND
        ${NV_PACKMAN_EXE}
        "install"
        "-q"
        "-l"
        "_build/target-deps/openssl"
        ${OPENSSL_PACKAGE_NAME}
        ${OPENSSL_PACKAGE_VERSION}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../../..
        COMMAND_ERROR_IS_FATAL ANY)
endif()

function (setup_libcurl_targets)
    if (NOT TARGET libcurl::libcurl)
        add_library(libcurl::libcurl STATIC IMPORTED)
        set_property(TARGET libcurl::libcurl APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG RELEASE RELWITHDEBINFO)
        set_target_properties(libcurl::libcurl PROPERTIES
            INTERFACE_COMPILE_DEFINITIONS
                CURL_STATICLIB)
        if (WIN32)
            set_target_properties(libcurl::libcurl PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/include"
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/include"
                IMPORTED_IMPLIB_DEBUG "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/lib/libcurl.lib"
                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/lib/libcurl.lib"
                IMPORTED_IMPLIB_RELEASE "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/lib/libcurl.lib"
                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/lib/libcurl.lib"
                IMPORTED_IMPLIB_RELWITHDEBINFO "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/lib/libcurl.lib"
                IMPORTED_LOCATION_RELWITHDEBINFO "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/lib/libcurl.lib")
            set_target_properties(libcurl::libcurl PROPERTIES
                INTERFACE_LINK_LIBRARIES
                    crypt32)
        else()
            set_target_properties(libcurl::libcurl PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/include"
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/include"
                IMPORTED_IMPLIB_DEBUG "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/lib/libcurl.a"
                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/lib/libcurl.a"
                IMPORTED_IMPLIB_RELEASE "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/lib/libcurl.a"
                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/lib/libcurl.a"
                IMPORTED_IMPLIB_RELWITHDEBINFO "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/lib/libcurl.a"
                IMPORTED_LOCATION_RELWITHDEBINFO "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/libcurl/lib/libcurl.a")
            set_target_properties(libcurl::libcurl PROPERTIES
                INTERFACE_LINK_LIBRARIES
                    openssl::crypto)
        endif()
    endif()
endfunction()

function (setup_zlib_targets)
    if (NOT TARGET zlib::zlib)
        add_library(zlib::zlib SHARED IMPORTED)
        set_property(TARGET zlib::zlib APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG RELEASE RELWITHDEBINFO)
        if (WIN32)
            set_target_properties(zlib::zlib PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/include"
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/include"
                IMPORTED_IMPLIB_DEBUG "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/lib/rt_dynamic/release/zlib.lib"
                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/lib/rt_dynamic/release/zlib.dll"
                IMPORTED_IMPLIB_RELEASE "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/lib/rt_dynamic/release/zlib.lib"
                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/lib/rt_dynamic/release/zlib.dll"
                IMPORTED_IMPLIB_RELWITHDEBINFO "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/lib/rt_dynamic/release/zlib.lib"
                IMPORTED_LOCATION_RELWITHDEBINFO "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/lib/rt_dynamic/release/zlib.dll")
        else()
            set_target_properties(zlib::zlib PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/include"
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/include"
                IMPORTED_IMPLIB_DEBUG "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/lib/libz.so"
                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/lib/libz.so"
                IMPORTED_IMPLIB_RELEASE "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/lib/libz.so"
                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/lib/libz.so"
                IMPORTED_IMPLIB_RELWITHDEBINFO "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/lib/libz.so"
                IMPORTED_LOCATION_RELWITHDEBINFO "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/zlib/lib/libz.so")
        endif()
    endif()
endfunction()

function (setup_openssl_targets)
    if (NOT TARGET openssl::crypto)
        add_library(openssl::crypto STATIC IMPORTED)
        set_target_properties(openssl::crypto PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/openssl/include"
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/openssl/include"
                IMPORTED_IMPLIB_DEBUG "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/openssl/lib/libcrypto.a"
                IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/openssl/lib/libcrypto.a"
                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/openssl/lib/libcrypto.a"
                IMPORTED_IMPLIB_RELEASE "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/openssl/lib/libcrypto.a"
                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/openssl/lib/libcrypto.a"
                IMPORTED_IMPLIB_RELWITHDEBINFO "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/openssl/lib/libcrypto.a"
                IMPORTED_LOCATION_RELWITHDEBINFO "${CMAKE_CURRENT_LIST_DIR}/../../../../_build/target-deps/openssl/lib/libcrypto.a")
    endif()
endfunction()

if (NOT WIN32)
    setup_openssl_targets()
endif()

setup_libcurl_targets()
setup_zlib_targets()
