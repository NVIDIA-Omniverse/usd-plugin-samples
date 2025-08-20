function (setup_imath_info)
#[============================================================[.rst:
    setup_imath_info
    ---------------
    Defines the Imath targets for a vendored OpenUSD dependency.

    Inputs
    ^^^^^^
    If the vendored targets are required to be defined, this method
    requires the following cmake variables to be defined:
    * PXR_CMAKE_DIR: The root directory of the OpenUSD build being used
      This is typically defined by OpenUSD itself and comes from
      pxrConfig.cmake via a find(pxr REQUIRED) invocation
    
    Targets
    ^^^^^^^
    This method will define the following targets:
    * Imath::Imath
    * Imath::ImathConfig

    If the Imath:* targets required are already defined (e.g., by a cmake toolchain,
    by performing a find(Imath required), etc.) this method will not redefine them.
    #]============================================================]

    # check if Imath was found by looking at the target we require
    if (NOT Imath_INCLUDE_DIR)
        # the include directory will be the same for any platform
        set(Imath_INCLUDE_DIR "${PXR_CMAKE_DIR}/include")
    endif()
    
    if(NOT TARGET Imath::ImathConfig)
        # header only target
        add_library(Imath::ImathConfig SHARED IMPORTED)
        set_property(TARGET Imath::ImathConfig APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG RELEASE RELWITHDEBINFO)
        set_target_properties(Imath::ImathConfig PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${Imath_INCLUDE_DIR}
            INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${Imath_INCLUDE_DIR})
    endif()

    if (NOT TARGET Imath::Imath)
        add_library(Imath::Imath SHARED IMPORTED)
        set_property(TARGET Imath::Imath APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG RELEASE RELWITHDEBINFO)

        # acquire the version of Imath we are vendoring
        set(Imath_CONFIG_H_FILE ${PXR_CMAKE_DIR}/include/imath/ImathConfig.h)
        if (EXISTS ${Imath_CONFIG_H_FILE})
            file(STRINGS ${Imath_CONFIG_H_FILE} Imath_CONFIG_H_FILE_LINES)
            foreach(line ${Imath_CONFIG_H_FILE_LINES})
                string(FIND "${line}" "IMATH_VERSION_MAJOR" result)
                if (NOT result EQUAL -1)
                    # found major version
                    math(EXPR result "${result} + 20")
                    string(SUBSTRING ${line} ${result} -1 line)
                    set(IMATH_VERSION_MAJOR ${line})
                    continue()
                endif()
                string(FIND "${line}" "IMATH_VERSION_MINOR" result)
                if (NOT result EQUAL -1)
                    # found minor version
                    math(EXPR result "${result} + 20")
                    string(SUBSTRING ${line} ${result} -1 line)
                    set(IMATH_VERSION_MINOR ${line})
                    break()
                endif()
            endforeach()
        endif()

        if (NOT DEFINED IMATH_VERSION_MAJOR)
            message(FATAL_ERROR "Unable to determine vendored version of Imath")
        endif()

        # search for the library
        set (Imath_LIBRARY_NAME "Imath-${IMATH_VERSION_MAJOR}_${IMATH_VERSION_MINOR}")
        find_library(Imath_LIBRARY_RELEASE ${Imath_LIBRARY_NAME}
            HINTS "${PXR_CMAKE_DIR}/lib" "${PXR_CMAKE_DIR}/bin")

        if (NOT Imath_LIBRARY_RELEASE)
            # can't find the libraries
            message(FATAL_ERROR "Unable to determine location of ${Imath_LIBRARY_NAME}")
        endif()

        if (WIN32)
            find_file(Imath_IMPLIB_LIBRARY "${Imath_LIBRARY_NAME}.lib"
                HINTS "${PXR_CMAKE_DIR}/lib" "${PXR_CMAKE_DIR}/bin")
            if (NOT Imath_IMPLIB_LIBRARY)
                message(FATAL_ERROR "Uanble to determine location of ${Imath_LIBRARY_NAME} import library")
            endif()
            set(Imath_IMPLIB_FILE ${Imath_IMPLIB_LIBRARY})
        else()
            set(Imath_IMPLIB_FILE ${Imath_LIBRARY_RELEASE})
        endif()

        set_target_properties(Imath::Imath PROPERTIES
            INTERFACE_COMPILE_DEFINITIONS "IMATH_DLL"
            INTERFACE_INCLUDE_DIRECTORIES ${Imath_INCLUDE_DIR}
            INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${Imath_INCLUDE_DIR}
            IMPORTED_IMPLIB_RELEASE ${Imath_IMPLIB_FILE}
            IMPORTED_LOCATION_RELEASE ${Imath_LIBRARY_RELEASE}
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
            MAP_IMPORTED_CONFIG_DEBUG Release)
    endif()
endfunction()

# make sure we can locate and define targets for Imath
if (NOT Imath_FOUND)
    setup_imath_info()
    set(Imath_FOUND 1)
endif()
