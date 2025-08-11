function (setup_tbb_info)
#[============================================================[.rst:
    setup_tbb_info
    ---------------
    Determines if the TBB targets required for OpenUSD
    are defined and if not, defines those targets using some
    hueristics based on the location of the OpenUSD build being used.

    Inputs
    ^^^^^^
    If TBB targets are required to be defined, this method requires
    the following cmake variables to be defined:
    * PXR_CMAKE_DIR: The root directory of the OpenUSD build being used
      This is typically defined by OpenUSD itself and comes from
      pxrConfig.cmake via a find(pxr REQUIRED) invocation
    
    Targets
    ^^^^^^^
    This method will define the following imported library and
    associated target definitions:
    * TBB:tbb
    *   INTERFACE_COMPILE_DEFINITIONS
    *   INTERFACE_INCLUDE_DIRECTORIES
    *   INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
    *   IMPORTED_IMPLIB_RELEASE
    *   IMPORTED_LOCATION_RELEASE
    *   IMPORTED_IMPLIB_DEBUG
    *   IMPORTED_LOCATION_DEBUG
    *   MAP_IMPORTED_CONFIG_RELWITHDEBUGINFO

    If the TBB:* targets required are already defined (e.g., by a cmake toolchain,
    by performing a find(TBB required), etc.) this method will not redefine them.
    #]============================================================]

    # check if TBB was found by looking at the target we require
    if(NOT TARGET TBB::tbb)
        # define the TBB::tbb target which is required as
        # a dependency of the OpenUSD imported targets
        # NOTE: Some OpenUSD builds will output tbb include / lib
        # paths as absolute paths on the machine and some will output
        # a link to the target, so we define the target here always
        # just in case we need it referenced
        add_library(TBB::tbb SHARED IMPORTED)
        set_property(TARGET TBB::tbb APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG RELEASE RELWITHDEBINFO)

        # the include directory will be the same for any platform
        set(TBB_INCLUDE_DIR "${PXR_CMAKE_DIR}/include")

        # now search for the libraries
        find_library(TBB_LIBRARY_RELEASE "tbb"
            HINTS "${PXR_CMAKE_DIR}/lib" "${PXR_CMAKE_DIR}/bin")

        if(NOT TBB_LIBRARY_RELEASE)
            # unfortunately we can't find the the tbb libraries so we can't
            # set the imported properties correctly
            message(FATAL_ERROR "Unable to determine location of TBB libraries!")
        endif()

        if(WIN32)
            find_file(TBB_IMPLIB_LIBRARY "tbb.lib"
                HINTS "${PXR_CMAKE_DIR}/lib" "{PXR_CMAKE_DIR}/bin")
            if (NOT TBB_IMPLIB_LIBRARY)
                message(FATAL_ERROR "Unable to determine location of TBB imported libraries!")
            endif()
            set(TBB_IMPLIB_FILE ${TBB_IMPLIB_LIBRARY})
        else()
            set(TBB_IMPLIB_FILE ${TBB_LIBRARY_RELEASE})
        endif()

        # the TBB debug libraries should be in the same location as the release
        cmake_path(GET TBB_LIBRARY_RELEASE PARENT_PATH TBB_LIBRARY_DIR)
        set(TBB_LIBRARY_DEBUG "${TBB_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}tbb_debug${CMAKE_SHARED_LIBRARY_SUFFIX}")

        set_target_propertieS(TBB::tbb PROPERTIES
                INTERFACE_COMPILE_DEFINITIONS "$<$<CONFIG:Debug>:TBB_USE_DEBUG=1>"
                INTERFACE_INCLUDE_DIRECTORIES ${TBB_INCLUDE_DIR}
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${TBB_INCLUDE_DIR}
                IMPORTED_IMPLIB_RELEASE ${TBB_IMPLIB_FILE}
                IMPORTED_IMPLIB_DEBUG ${TBB_IMPLIB_FILE}
                IMPORTED_LOCATION_RELEASE ${TBB_LIBRARY_RELEASE}
                IMPORTED_LOATION_DEBUG ${TBB_LIBRARY_DEBUG}
                MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
    endif()

endfunction()

# make sure we can locate and define targets for TBB
if (NOT TBB_FOUND)
    setup_tbb_info()
    set(TBB_FOUND 1)
endif()
