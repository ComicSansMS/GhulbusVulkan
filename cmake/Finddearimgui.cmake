
find_path(DEAR_IMGUI_ROOT_DIR NAMES imgui.cpp
    HINTS
    ${PROJECT_BINARY_DIR}/dependencies/imgui/install
    ${DEAR_IMGUI_ROOT}
    NO_DEFAULT_PATH
)
mark_as_advanced(DEAR_IMGUI_ROOT_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(dearimgui REQUIRED_VARS DEAR_IMGUI_ROOT_DIR)

if(dearimgui_FOUND)
    if(NOT TARGET dearimgui::dearimgui)
        add_library(dearimgui STATIC
            ${DEAR_IMGUI_ROOT_DIR}/imconfig.h
            ${DEAR_IMGUI_ROOT_DIR}/imgui.cpp
            ${DEAR_IMGUI_ROOT_DIR}/imgui.h
            ${DEAR_IMGUI_ROOT_DIR}/imgui_demo.cpp
            ${DEAR_IMGUI_ROOT_DIR}/imgui_draw.cpp
            ${DEAR_IMGUI_ROOT_DIR}/imgui_internal.h
            ${DEAR_IMGUI_ROOT_DIR}/imgui_widgets.cpp
            ${DEAR_IMGUI_ROOT_DIR}/imstb_rectpack.h
            ${DEAR_IMGUI_ROOT_DIR}/imstb_textedit.h
            ${DEAR_IMGUI_ROOT_DIR}/imstb_truetype.h
            ${DEAR_IMGUI_ROOT_DIR}/examples/imgui_impl_glfw.cpp
            ${DEAR_IMGUI_ROOT_DIR}/examples/imgui_impl_glfw.h
            ${DEAR_IMGUI_ROOT_DIR}/examples/imgui_impl_vulkan.cpp
            ${DEAR_IMGUI_ROOT_DIR}/examples/imgui_impl_vulkan.h
        )
        target_include_directories(dearimgui PUBLIC
            ${DEAR_IMGUI_ROOT_DIR}
            ${DEAR_IMGUI_ROOT_DIR}/examples
        )
        target_link_libraries(dearimgui PUBLIC glfw Vulkan::Vulkan)
    endif()
endif()
