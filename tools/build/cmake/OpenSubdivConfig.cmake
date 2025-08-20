function (setup_opensubdiv_info)
#[============================================================[.rst:
    setup_opensubdiv_info
    ---------------
    Defines the OpenSubdiv targets for a vendored OpenUSD dependency.

    Inputs
    ^^^^^^
    If the vendored targets are required to be defined, this method
    requires the following cmake variables to be defined:
    * PXR_CMAKE_DIR: The root directory of the OpenUSD build being used
      This is typically defined by OpenUSD itself and comes from
      pxrConfig.cmake via a find(pxr REQUIRED) invocation
    
    Targets
    ^^^^^^^
    This method will define the following targets for linux:
    * OpenSubdiv::osdcpu
    * OpenSubdiv::osdgpu

    And the following targets for windows:
    * OpenSubdiv::osdcpu_static
    * OpenSubdiv::osdgpu_static

    If the OpenSubdiv:* targets required are already defined (e.g., by a cmake toolchain,
    by performing a find(OpenSubdiv required), etc.) this method will not redefine them.
    #]============================================================]

    # check if OpenSubdiv was found by looking at the target we require
    if (NOT OpenSubdiv_INCLUDE_DIR)
        # the include directory will be the same for any platform
        set(OpenSubdiv_INCLUDE_DIR "${PXR_CMAKE_DIR}/include")
    endif()
    
    if (WIN32)
        if (NOT TARGET OpenSubdiv::osdcpu_static)
            add_library(OpenSubdiv::osdcpu_static STATIC IMPORTED)
            set_property(TARGET OpenSubdiv::osdcpu_static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG RELEASE RELWITHDEBINFO)

            find_file(OpenSubdiv_LIBRARY "osdCPU.lib"
                HINTS "${PXR_CMAKE_DIR}/lib" "${PXR_CMAKE_DIR}/bin")
            if (NOT OpenSubdiv_LIBRARY)
                message(FATAL_ERROR "Uanble to determine location of osdCPU.lib")
            endif()

            set_target_properties(OpenSubdiv::osdcpu_static PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${OpenSubdiv_INCLUDE_DIR}
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${OpenSubdiv_INCLUDE_DIR}
                IMPORTED_LOCATION_RELEASE ${OpenSubdiv_LIBRARY}
                MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
                MAP_IMPORTED_CONFIG_DEBUG Release)
        endif()

        if (NOT TARGET OpenSubdiv::osdgpu_static)
            add_library(OpenSubdiv::osdgpu_static STATIC IMPORTED)
            set_property(TARGET OpenSubdiv::osdgpu_static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG RELEASE RELWITHDEBINFO)

            find_file(OpenSubdiv_GPU_LIBRARY "osdGPU.lib"
                HINTS "${PXR_CMAKE_DIR}/lib" "${PXR_CMAKE_DIR}/bin")
            if (NOT OpenSubdiv_GPU_LIBRARY)
                message(FATAL_ERROR "Uanble to determine location of osdGPU.lib")
            endif()

            set_target_properties(OpenSubdiv::osdgpu_static PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${OpenSubdiv_INCLUDE_DIR}
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${OpenSubdiv_INCLUDE_DIR}
                IMPORTED_LOCATION_RELEASE ${OpenSubdiv_GPU_LIBRARY}
                MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
                MAP_IMPORTED_CONFIG_DEBUG Release)
        endif()
    else()
        if(NOT TARGET OpenSubdiv::osdcpu)
            add_library(OpenSubdiv::osdcpu SHARED IMPORTED)
            set_property(TARGET OpenSubdiv::osdcpu APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG RELEASE RELWITHDEBINFO)

            find_library(OpenSubdiv_LIBRARY "osdCPU"
                HINTS "${PXR_CMAKE_DIR}/lib" "${PXR_CMAKE_DIR}/bin")
            if (NOT OpenSubdiv_LIBRARY)
                message(FATAL_ERROR "Uanble to determine location of osdCPU.so")
            endif()

            set_target_properties(OpenSubdiv::osdcpu PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${OpenSubdiv_INCLUDE_DIR}
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${OpenSubdiv_INCLUDE_DIR}
                IMPORTED_LOCATION_RELEASE ${OpenSubdiv_LIBRARY}
                MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
                MAP_IMPORTED_CONFIG_DEBUG Release)
        endif()

        if(NOT TARGET OpenSubdiv::osdgpu)
            add_library(OpenSubdiv::osdgpu SHARED IMPORTED)
            set_property(TARGET OpenSubdiv::osdgpu APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG RELEASE RELWITHDEBINFO)

            find_library(OpenSubdiv_GPU_LIBRARY "osdGPU"
                HINTS "${PXR_CMAKE_DIR}/lib" "${PXR_CMAKE_DIR}/bin")
            if (NOT OpenSubdiv_GPU_LIBRARY)
                message(FATAL_ERROR "Uanble to determine location of osdGPU.so")
            endif()

            set_target_properties(OpenSubdiv::osdgpu PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${OpenSubdiv_INCLUDE_DIR}
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${OpenSubdiv_INCLUDE_DIR}
                IMPORTED_LOCATION_RELEASE ${OpenSubdiv_GPU_LIBRARY}
                MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
                MAP_IMPORTED_CONFIG_DEBUG Release)
        endif()
    endif()
endfunction()

# make sure we can locate and define targets for OpenSubdiv
if (NOT OpenSubdiv_FOUND)
    setup_opensubdiv_info()
    set(OpenSubdiv_FOUND 1)
endif()
