
find_path(STB_INCLUDE_DIR NAMES stb_image.h
    HINTS
    ${PROJECT_BINARY_DIR}/dependencies/stb/install
    ${STB_ROOT}
    PATH_SUFFIXES src
    NO_DEFAULT_PATH
)
mark_as_advanced(STB_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(stb REQUIRED_VARS STB_INCLUDE_DIR)

if(stb_FOUND)
    if(NOT TARGET stb::stb)
        add_library(stb::stb INTERFACE IMPORTED)
        target_include_directories(stb::stb INTERFACE
            ${STB_INCLUDE_DIR})
    endif()
endif()
