
set(FIND_SPIRV_CROSS_HINTS
    ${SPIRV_CROSS_ROOT}
    ${PROJECT_BINARY_DIR}/dependencies/spirv-cross/install
    $ENV{spirv_cross_ROOT}
    $ENV{spirv_cross_core_ROOT}
)

find_package(spirv_cross_core HINTS ${FIND_SPIRV_CROSS_HINTS})
find_package(spirv_cross_reflect HINTS ${FIND_SPIRV_CROSS_HINTS})
#find_package(spirv_cross_glsl HINTS ${FIND_SPIRV_CROSS_HINTS})
#find_package(spirv_cross_cpp HINTS ${FIND_SPIRV_CROSS_HINTS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(spirv-cross REQUIRED_VARS spirv_cross_reflect_DIR)

if(spirv-cross_FOUND)
    target_link_libraries(spirv-cross-reflect INTERFACE spirv-cross-core)
endif()
