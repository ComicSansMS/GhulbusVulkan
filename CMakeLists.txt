cmake_minimum_required(VERSION 3.14)

project(GhulbusVulkan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

###############################################################################
## Build Options
###############################################################################

if($ENV{TRAVIS})
    if($ENV{TRAVIS} STREQUAL "true")
        message(STATUS "Building on Travis-CI.")
        set(IS_TRAVIS_BUILD TRUE)
    endif()
endif()

option(GB_GENERATE_COVERAGE_INFO "If set, line coverage info will be generated from debug test runs." OFF)
if(GB_GENERATE_COVERAGE_INFO)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # this is an undocumented option for changing the output extension from *.cpp.ext
        # to *.ext; As there is no reasonable way to obtain the list of object files
        # this seems to be the best option to generate the naming gcov expects.
        set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
        set(GCOV_COMMAND gcov CACHE STRING "Command to invoke to run gcov.")
    endif()
endif()

option(GB_AUTO_BUILD_DEPENDENCIES "If set, dependencies will be built automatically as part of the initial configuration" OFF)
if(GB_AUTO_BUILD_DEPENDENCIES)
    find_package(Git REQUIRED)
    if(NOT EXISTS ${PROJECT_BINARY_DIR}/dependencies/build_dependencies.cmake)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} clone https://github.com/ComicSansMS/GhulbusVulkanDependencies.git
                    ${PROJECT_BINARY_DIR}/dependencies
            RESULT_VARIABLE result
        )
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Error cloning dependencies repo.")
        endif()
        message("Building dependencies (this may take a while)...")
        list(APPEND options -DGENERATOR=${CMAKE_GENERATOR})
        if(CMAKE_GENERATOR_TOOLSET)
            list(APPEND options )
        endif()
        if(CMAKE_GENERATOR_PLATFORM)
        endif()
        execute_process(COMMAND ${CMAKE_COMMAND}
            -DGENERATOR=${CMAKE_GENERATOR} -DPLATFORM=${CMAKE_GENERATOR_PLATFORM}
            -DTOOLSET=${CMAKE_GENERATOR_TOOLSET} -P build_dependencies.cmake
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/dependencies
            OUTPUT_QUIET
            RESULT_VARIABLE result
        )
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Error building dependencies.")
        endif()
    endif()
endif()

if(MSVC)
    option(GB_PARALLEL_BUILD "If set, uses multiple build processes in parallel." ON)
    if(GB_PARALLEL_BUILD)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif()
endif()

option(BUILD_SHARED_LIBS "If checked, generate dynamic instead of static libraries." OFF)

###############################################################################
## Dependencies
###############################################################################

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(Vulkan REQUIRED)
find_program(Vulkan_GLSLC NAMES glslc HINTS ${Vulkan_INCLUDE_DIR}/../bin ENV GLSLC_EXECUTABLE)
if(NOT Vulkan_GLSLC)
    message(SEND_ERROR "Cannot find glslc compiler. Provide may provide a search hint via the GLSLC_EXECUTABLE environment variable.")
endif()
function(add_shader target_name output_file source_file)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs COMPILE_OPTIONS)
    cmake_parse_arguments(ADD_SHADER_TARGET "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )
    add_custom_command(
        OUTPUT ${output_file}
        COMMAND ${Vulkan_GLSLC} ${ADD_SHADER_TARGET_COMPILE_OPTIONS} -o ${output_file} ${source_file}
        COMMENT "Compiling shader ${source_file}"
        MAIN_DEPENDENCY ${source_file}
    )
    target_sources(${target_name} PRIVATE ${output_file})
endfunction()

find_package(glfw3 REQUIRED HINTS ${PROJECT_BINARY_DIR}/dependencies/glfw3/install)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(VulkanMemoryAllocator REQUIRED)

set(GHULBUS_BASE_ROOT "" CACHE PATH "GhulbusBase installation root directory.")
if(GHULBUS_BASE_ROOT)
    list(APPEND CMAKE_PREFIX_PATH ${GHULBUS_BASE_ROOT}/cmake)
endif()
find_package(gbBase REQUIRED)
if(NOT ((GB_BASE_IS_SHARED AND BUILD_SHARED_LIBS) OR ((NOT GB_BASE_IS_SHARED) AND (NOT BUILD_SHARED_LIBS))))
    message(STATUS "gbBase is shared " ${GB_BASE_IS_SHARED} "; gbVk is shared " ${BUILD_SHARED_LIBS} ".")
    message(FATAL_ERROR "Mixing shared and static builds of Ghulbus libraries is not supported.")
endif()
find_package(gbMath REQUIRED)
get_property(gbMath_SOURCES TARGET Ghulbus::gbMath PROPERTY INTERFACE_SOURCES)
source_group(gbMath FILES ${gbMath_SOURCES})

###############################################################################
## Library gbVk
###############################################################################

set(GB_VK_VERSION_MAJOR 0)
set(GB_VK_VERSION_MINOR 1)
set(GB_VK_VERSION_PATCH 0)
set(GB_VK_VERSION "${GB_VK_VERSION_MAJOR}.${GB_VK_VERSION_MINOR}.${GB_VK_VERSION_PATCH}")

set(GB_VK_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(GB_VK_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(GB_VK_TEST_DIR ${PROJECT_SOURCE_DIR}/test)

set(GB_VK_SOURCE_FILES
    ${GB_VK_SOURCE_DIR}/Buffer.cpp
    ${GB_VK_SOURCE_DIR}/CommandBuffer.cpp
    ${GB_VK_SOURCE_DIR}/CommandBuffers.cpp
    ${GB_VK_SOURCE_DIR}/CommandPool.cpp
    ${GB_VK_SOURCE_DIR}/DescriptorPool.cpp
    ${GB_VK_SOURCE_DIR}/DescriptorPoolBuilder.cpp
    ${GB_VK_SOURCE_DIR}/DescriptorSet.cpp
    ${GB_VK_SOURCE_DIR}/DescriptorSetLayout.cpp
    ${GB_VK_SOURCE_DIR}/DescriptorSetLayoutBuilder.cpp
    ${GB_VK_SOURCE_DIR}/DescriptorSets.cpp
    ${GB_VK_SOURCE_DIR}/Device.cpp
    ${GB_VK_SOURCE_DIR}/DeviceBuilder.cpp
    ${GB_VK_SOURCE_DIR}/DeviceMemory.cpp
    ${GB_VK_SOURCE_DIR}/Fence.cpp
    ${GB_VK_SOURCE_DIR}/Framebuffer.cpp
    ${GB_VK_SOURCE_DIR}/Image.cpp
    ${GB_VK_SOURCE_DIR}/ImageView.cpp
    ${GB_VK_SOURCE_DIR}/Instance.cpp
    ${GB_VK_SOURCE_DIR}/PhysicalDevice.cpp
    ${GB_VK_SOURCE_DIR}/Pipeline.cpp
    ${GB_VK_SOURCE_DIR}/PipelineBuilder.cpp
    ${GB_VK_SOURCE_DIR}/PipelineLayout.cpp
    ${GB_VK_SOURCE_DIR}/PipelineLayoutBuilder.cpp
    ${GB_VK_SOURCE_DIR}/Queue.cpp
    ${GB_VK_SOURCE_DIR}/RenderPass.cpp
    ${GB_VK_SOURCE_DIR}/RenderPassBuilder.cpp
    ${GB_VK_SOURCE_DIR}/Sampler.cpp
    ${GB_VK_SOURCE_DIR}/Semaphore.cpp
    ${GB_VK_SOURCE_DIR}/ShaderModule.cpp
    ${GB_VK_SOURCE_DIR}/Spirv.cpp
    ${GB_VK_SOURCE_DIR}/StringConverters.cpp
    ${GB_VK_SOURCE_DIR}/SubmitStaging.cpp
    ${GB_VK_SOURCE_DIR}/Swapchain.cpp
)

set(GB_VK_HEADER_FILES
    ${GB_VK_INCLUDE_DIR}/gbVk/Buffer.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/CommandBuffer.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/CommandBuffers.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/CommandPool.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/config.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/DescriptorPool.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/DescriptorPoolBuilder.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/DescriptorSet.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/DescriptorSetLayout.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/DescriptorSetLayoutBuilder.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/DescriptorSets.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Device.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/DeviceBuilder.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/DeviceMemory.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Exceptions.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Fence.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/ForwardDecl.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Framebuffer.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Image.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/ImageView.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Instance.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/PhysicalDevice.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Pipeline.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/PipelineBuilder.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/PipelineLayout.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/PipelineLayoutBuilder.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Queue.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/RenderPass.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/RenderPassBuilder.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Sampler.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Semaphore.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/ShaderModule.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Spirv.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/StringConverters.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/SubmitStaging.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Swapchain.hpp
)

set(GB_VK_TEST_SOURCES
    ${GB_VK_TEST_DIR}/TestVulkan.cpp
    ${GB_VK_TEST_DIR}/TestPhysicalDevice.cpp
    ${GB_VK_TEST_DIR}/TestStringConverters.cpp
)

add_library(gbVk
    ${GB_VK_SOURCE_FILES}
    ${GB_VK_HEADER_FILES}
)
target_include_directories(gbVk PUBLIC $<BUILD_INTERFACE:${GB_VK_INCLUDE_DIR}>)
target_link_libraries(gbVk PUBLIC Threads::Threads)
target_link_libraries(gbVk PUBLIC Ghulbus::gbBase)
target_link_libraries(gbVk PUBLIC Vulkan::Vulkan)

if(NOT GB_GENERATE_COVERAGE_INFO)
    target_compile_definitions(gbVk PRIVATE $<$<CONFIG:Debug>:GHULBUS_CONFIG_ASSERT_LEVEL_DEBUG>)
endif()
target_compile_definitions(gbVk PUBLIC $<$<CXX_COMPILER_ID:MSVC>:BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE>)
target_compile_definitions(gbVk PUBLIC $<$<PLATFORM_ID:Windows>:GHULBUS_CONFIG_VULKAN_PLATFORM_WIN32>)
target_compile_options(gbVk PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4>)
target_compile_options(gbVk PUBLIC $<$<STREQUAL:$<CXX_COMPILER_ID>,MSVC>:/permissive->)
target_compile_options(gbVk PUBLIC $<$<STREQUAL:$<CXX_COMPILER_ID>,MSVC>:/std:c++latest>)
target_compile_options(gbVk PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>>:-pedantic -Wall>)
target_compile_options(gbVk PRIVATE $<$<CXX_COMPILER_ID:GNU>:-pedantic -Wall>)

include(GenerateExportHeader)
generate_export_header(gbVk
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/generated/gbVk/gbVk_Export.hpp
    PREFIX_NAME GHULBUS_LIBRARY_
)
set(GB_VK_GENERATED_HEADER_FILES ${PROJECT_BINARY_DIR}/generated/gbVk/gbVk_Export.hpp)
target_include_directories(gbVk PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>)
if(BUILD_SHARED_LIBS)
    set_property(TARGET gbVk PROPERTY CXX_VISIBILITY_PRESET hidden)
    set_property(TARGET gbVk PROPERTY VISIBILITY_INLINES_HIDDEN ON)
endif()

set_property(TARGET gbVk PROPERTY VERSION ${GB_VK_VERSION})
set_property(TARGET gbVk PROPERTY SOVERSION ${GB_VK_VERSION_MAJOR})
set_property(TARGET gbVk PROPERTY DEBUG_POSTFIX d)

# Copy dlls
if(WIN32 AND BUILD_SHARED_LIBS)
    set(src_genexp)
    foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${cfg} cfg_upper)
        set(src_genexp "${src_genexp}$<$<CONFIG:${cfg}>:$<TARGET_PROPERTY:Ghulbus::gbBase,IMPORTED_LOCATION_${cfg_upper}>>")
    endforeach()
    add_custom_command(TARGET gbVk POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src_genexp} $<TARGET_FILE_DIR:gbVk>)
endif()

###############################################################################
## gbGraphics
###############################################################################

set(GB_GRAPHICS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/gbGraphics)
set(GB_GRAPHICS_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(GB_GRAPHICS_TEST_DIR ${PROJECT_SOURCE_DIR}/test/gbGraphics)

set(GB_GRAPHICS_SOURCE_FILES
    ${GB_GRAPHICS_SOURCE_DIR}/CommandPoolRegistry.cpp
    ${GB_GRAPHICS_SOURCE_DIR}/Graphics.cpp
    ${GB_GRAPHICS_SOURCE_DIR}/GraphicsInstance.cpp
    ${GB_GRAPHICS_SOURCE_DIR}/Image2d.cpp
    ${GB_GRAPHICS_SOURCE_DIR}/ObjParser.cpp
    ${GB_GRAPHICS_SOURCE_DIR}/Window.cpp
)

set(GB_GRAPHICS_HEADER_FILES
    ${GB_GRAPHICS_INCLUDE_DIR}/gbGraphics/CommandPoolRegistry.hpp
    ${GB_GRAPHICS_INCLUDE_DIR}/gbGraphics/config.hpp
    ${GB_GRAPHICS_INCLUDE_DIR}/gbGraphics/Exceptions.hpp
    ${GB_GRAPHICS_INCLUDE_DIR}/gbGraphics/Graphics.hpp
    ${GB_GRAPHICS_INCLUDE_DIR}/gbGraphics/GraphicsInstance.hpp
    ${GB_GRAPHICS_INCLUDE_DIR}/gbGraphics/Image2d.hpp
    ${GB_GRAPHICS_INCLUDE_DIR}/gbGraphics/ObjParser.hpp
    ${GB_GRAPHICS_INCLUDE_DIR}/gbGraphics/Window.hpp
)

set(GB_GRAPHICS_DETAIL_SOURCE_FILES
    ${GB_GRAPHICS_SOURCE_DIR}/detail/DeviceMemoryAllocator.cpp
    ${GB_GRAPHICS_SOURCE_DIR}/detail/QueueSelection.cpp
    ${GB_GRAPHICS_SOURCE_DIR}/detail/VulkanMemoryAllocator.cpp
)
source_group("detail\\Source Files" FILES ${GB_GRAPHICS_DETAIL_SOURCE_FILES})

set(GB_GRAPHICS_DETAIL_HEADER_FILES
    ${GB_GRAPHICS_INCLUDE_DIR}/gbGraphics/detail/DeviceMemoryAllocator.hpp
    ${GB_GRAPHICS_INCLUDE_DIR}/gbGraphics/detail/QueueSelection.hpp
    ${GB_GRAPHICS_INCLUDE_DIR}/gbGraphics/detail/VulkanMemoryAllocator.hpp
)
source_group("detail\\Header Files" FILES ${GB_GRAPHICS_DETAIL_HEADER_FILES})

set(GB_GRAPHICS_TEST_SOURCES
    ${GB_GRAPHICS_TEST_DIR}/TestGraphics.cpp
    ${GB_GRAPHICS_TEST_DIR}/TestQueueSelection.cpp
)

add_library(gbGraphics
    ${GB_GRAPHICS_SOURCE_FILES}
    ${GB_GRAPHICS_HEADER_FILES}
    ${GB_GRAPHICS_DETAIL_SOURCE_FILES}
    ${GB_GRAPHICS_DETAIL_HEADER_FILES}
)
target_include_directories(gbGraphics PUBLIC $<BUILD_INTERFACE:${GB_GRAPHICS_INCLUDE_DIR}>)
target_link_libraries(gbGraphics PUBLIC Ghulbus::gbBase Ghulbus::gbMath gbVk glfw)
target_link_libraries(gbGraphics PRIVATE VulkanMemoryAllocator::VulkanMemoryAllocator)

if(NOT GB_GENERATE_COVERAGE_INFO)
    target_compile_definitions(gbGraphics PRIVATE $<$<CONFIG:Debug>:GHULBUS_CONFIG_ASSERT_LEVEL_DEBUG>)
endif()
target_compile_options(gbGraphics PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4>)
target_compile_options(gbGraphics PUBLIC $<$<STREQUAL:$<CXX_COMPILER_ID>,MSVC>:/permissive->)
target_compile_options(gbGraphics PUBLIC $<$<STREQUAL:$<CXX_COMPILER_ID>,MSVC>:/std:c++latest>)
target_compile_options(gbGraphics PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>>:-pedantic -Wall>)
target_compile_options(gbGraphics PRIVATE $<$<CXX_COMPILER_ID:GNU>:-pedantic -Wall>)

include(GenerateExportHeader)
generate_export_header(gbGraphics
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/generated/gbGraphics/gbGraphics_Export.hpp
    PREFIX_NAME GHULBUS_LIBRARY_
)
set(GB_GRAPHICS_GENERATED_HEADER_FILES ${PROJECT_BINARY_DIR}/generated/gbGraphics/gbGraphics_Export.hpp)
target_include_directories(gbGraphics PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>)
if(BUILD_SHARED_LIBS)
    set_property(TARGET gbGraphics PROPERTY CXX_VISIBILITY_PRESET hidden)
    set_property(TARGET gbGraphics PROPERTY VISIBILITY_INLINES_HIDDEN ON)
endif()

set_property(TARGET gbGraphics PROPERTY VERSION ${GB_VK_VERSION})
set_property(TARGET gbGraphics PROPERTY SOVERSION ${GB_VK_VERSION_MAJOR})
set_property(TARGET gbGraphics PROPERTY DEBUG_POSTFIX d)

# Copy dlls
if(WIN32 AND BUILD_SHARED_LIBS)
    set(src_genexp)
    foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${cfg} cfg_upper)
        set(src_genexp "${src_genexp}$<$<CONFIG:${cfg}>:$<TARGET_PROPERTY:Ghulbus::gbBase,IMPORTED_LOCATION_${cfg_upper}>>")
    endforeach()
    add_custom_command(TARGET gbGraphics POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src_genexp} $<TARGET_FILE_DIR:gbGraphics>)
endif()

###############################################################################
## Tools
###############################################################################
option(GB_VK_BUILD_TOOLS "Uncheck this if you do not want to build the GUI tools." ON)
if(GB_VK_BUILD_TOOLS)
    find_package(Qt5BaseDir)
    if(QT5_BASE_DIR)
        list(APPEND CMAKE_PREFIX_PATH ${QT5_BASE_DIR})
    endif()
    find_package(Qt5Widgets 5.8 REQUIRED)

    set(GB_VK_DEVICE_EXPLORER_DIR ${PROJECT_SOURCE_DIR}/tools/device_explorer)
    set(GB_VK_DEVICE_EXPLORER_QT_MOC_HEADER_FILES
        ${GB_VK_DEVICE_EXPLORER_DIR}/central_widget.hpp
        ${GB_VK_DEVICE_EXPLORER_DIR}/main_window.hpp
    )
    qt5_wrap_cpp(GB_VK_DEVICE_EXPLORER_QT_MOC_SOURCE_FILES ${GB_VK_DEVICE_EXPLORER_QT_MOC_HEADER_FILES})
    source_group(moc FILES ${GB_VK_DEVICE_EXPLORER_QT_MOC_SOURCE_FILES})
    add_executable(gb_vk_device_explorer
        ${GB_VK_DEVICE_EXPLORER_QT_MOC_HEADER_FILES}
        ${GB_VK_DEVICE_EXPLORER_QT_MOC_SOURCE_FILES}
        ${GB_VK_DEVICE_EXPLORER_DIR}/central_widget.cpp
        ${GB_VK_DEVICE_EXPLORER_DIR}/device_explorer.cpp
        ${GB_VK_DEVICE_EXPLORER_DIR}/main_window.cpp
    )
    target_include_directories(gb_vk_device_explorer PUBLIC ${GB_VK_DEVICE_EXPLORER_DIR})
    target_link_libraries(gb_vk_device_explorer PUBLIC gbVk Qt5::Widgets)
    if(WIN32)
        set_property(TARGET gb_vk_device_explorer PROPERTY WIN32_EXECUTABLE TRUE)
        target_link_libraries(gb_vk_device_explorer PUBLIC Qt5::WinMain)

        getQt5Dlls(Qt5::Widgets qt_DLLS)
        file(COPY ${qt_DLLS} DESTINATION ${PROJECT_BINARY_DIR})
    endif()
endif()

###############################################################################
## Testing gbVk
###############################################################################

option(GB_BUILD_TESTS "Determines whether to build tests." ON)
if(GB_BUILD_TESTS)
    enable_testing()

    if(NOT TARGET Catch)
        include(ExternalProject)
        if(IS_TRAVIS_BUILD)     # on travis, use git for fetching instead of wget
            set(GB_FETCH_EXTERNAL_CATCH
                GIT_REPOSITORY https://github.com/philsquared/Catch.git
                GIT_TAG v2.1.2)
        elseif(WIN32)
            set(GB_FETCH_EXTERNAL_CATCH
                URL https://github.com/catchorg/Catch2/archive/v2.1.2.zip
                URL_HASH MD5=54d2785d7e5549907421e8db5c1b3817)
        else()
            set(GB_FETCH_EXTERNAL_CATCH
                URL https://github.com/catchorg/Catch2/archive/v2.1.2.tar.gz
                URL_HASH MD5=4c5740d7446d9b4e3735dcff0951aaca)
        endif()
        ExternalProject_Add(Catch-External
            PREFIX ${CMAKE_BINARY_DIR}/external/Catch
            ${GB_FETCH_EXTERNAL_CATCH}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/external/Catch/src/Catch-External/single_include/catch.hpp
                                                     ${CMAKE_BINARY_DIR}/external/Catch/include/catch.hpp
        )
        add_library(Catch INTERFACE)
        add_dependencies(Catch Catch-External)
        target_include_directories(Catch INTERFACE ${CMAKE_BINARY_DIR}/external/Catch/include)
    endif()

    add_executable(gbVk_Test ${GB_VK_TEST_SOURCES})
    target_link_libraries(gbVk_Test gbVk Catch)
    add_test(NAME TestVulkan COMMAND gbVk_Test)

    add_executable(gbGraphics_Test ${GB_GRAPHICS_TEST_SOURCES})
    target_link_libraries(gbGraphics_Test gbGraphics Catch)
    add_test(NAME TestGraphics COMMAND gbGraphics_Test)

    if(GB_GENERATE_COVERAGE_INFO AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
        target_compile_options(gbVk PRIVATE --coverage -fprofile-arcs -ftest-coverage)
        target_compile_definitions(gbVk PRIVATE GHULBUS_CONFIG_ASSERT_LEVEL_PRODUCTION)
        target_link_libraries(gbVk PRIVATE -fprofile-arcs -ftest-coverage)
        set(COVERAGE_SOURCES ${GB_VK_SOURCE_FILES})
        add_custom_target(gbVk_TestCoverage
            COMMAND ${GCOV_COMMAND} -o CMakeFiles/gbVk.dir/src ${COVERAGE_SOURCES}
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
            COMMENT "Collecting gcov coverage information"
        )
    endif()
endif()

add_executable(demo ${PROJECT_SOURCE_DIR}/demo/demo.cpp ${PROJECT_SOURCE_DIR}/demo/external/stb_image_impl.cpp)
target_link_libraries(demo PUBLIC gbVk glfw Ghulbus::gbMath)
target_include_directories(demo PUBLIC ${PROJECT_SOURCE_DIR}/demo)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT demo)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/shaders)
add_shader(demo ${PROJECT_BINARY_DIR}/shaders/simple_compute.spv ${PROJECT_SOURCE_DIR}/demo/shaders/simple.comp)
add_shader(demo ${PROJECT_BINARY_DIR}/shaders/vert_hardcoded.spv ${PROJECT_SOURCE_DIR}/demo/shaders/demo_hardcoded.vert)
add_shader(demo ${PROJECT_BINARY_DIR}/shaders/vert_direct.spv ${PROJECT_SOURCE_DIR}/demo/shaders/demo_direct_vertices.vert)
add_shader(demo ${PROJECT_BINARY_DIR}/shaders/vert_mvp.spv ${PROJECT_SOURCE_DIR}/demo/shaders/demo_ubo_mvp.vert COMPILE_OPTIONS -O)
add_shader(demo ${PROJECT_BINARY_DIR}/shaders/vert_textured.spv ${PROJECT_SOURCE_DIR}/demo/shaders/demo_textured.vert COMPILE_OPTIONS -O)
add_shader(demo ${PROJECT_BINARY_DIR}/shaders/frag_hardcoded.spv ${PROJECT_SOURCE_DIR}/demo/shaders/demo_hardcoded.frag)
add_shader(demo ${PROJECT_BINARY_DIR}/shaders/frag_textured.spv ${PROJECT_SOURCE_DIR}/demo/shaders/demo_textured.frag)

file(COPY ${PROJECT_SOURCE_DIR}/demo/textures/statue.jpg DESTINATION ${PROJECT_BINARY_DIR}/textures)

add_executable(demo_graphics ${PROJECT_SOURCE_DIR}/demo/demo_graphics.cpp ${PROJECT_SOURCE_DIR}/demo/external/stb_image_impl.cpp)
target_link_libraries(demo_graphics PUBLIC gbGraphics)
target_include_directories(demo_graphics PUBLIC ${PROJECT_SOURCE_DIR}/demo)

###############################################################################
## Doxygen gbVk
###############################################################################

option(GB_GENERATE_DOXYGEN_DOCUMENTATION "If checked, API documentation will be generated if Doxygen is available." ON)
if(GB_GENERATE_DOXYGEN_DOCUMENTATION)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(GB_LIB_DOC_DIR ${PROJECT_BINARY_DIR}/doc)
        configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in ${GB_LIB_DOC_DIR}/Doxyfile @ONLY)
        add_custom_target(doxygen-doc
            COMMAND ${DOXYGEN_EXECUTABLE}
            WORKING_DIRECTORY ${GB_LIB_DOC_DIR}
            COMMENT "Building doxygen documentation"
            SOURCES ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in
        )
        source_group("" FILES ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in)
    endif()
endif()


###############################################################################
## Packaging gbVk
###############################################################################

set(GB_LIBRARY_NAMESPACE Ghulbus::)

install(TARGETS gbVk EXPORT gbVkTargets
    RUNTIME DESTINATION bin/$<CONFIG>
    LIBRARY DESTINATION lib/$<CONFIG>
    ARCHIVE DESTINATION lib/$<CONFIG>
    INCLUDES DESTINATION include
)
target_include_directories(gbVk PUBLIC $<INSTALL_INTERFACE:include>)
install(FILES ${GB_VK_HEADER_FILES} ${GB_VK_GENERATED_HEADER_FILES} DESTINATION include/gbVk)
if(MSVC AND BUILD_SHARED_LIBS)
    install(FILES $<TARGET_PDB_FILE:gbVk> DESTINATION bin/Debug CONFIGURATIONS Debug)
    install(FILES $<TARGET_PDB_FILE:gbVk> DESTINATION bin/RelWithDebInfo CONFIGURATIONS RelWithDebInfo)
endif()
install(FILES LICENSE DESTINATION ".")

if(GB_GENERATE_DOXYGEN_DOCUMENTATION AND DOXYGEN_FOUND)
    install(DIRECTORY ${GB_LIB_DOC_DIR}/html DESTINATION doc MESSAGE_NEVER)
endif()

include(CMakePackageConfigHelpers)
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/gbVkConfig.cmake.in
    ${PROJECT_BINARY_DIR}/cmake/gbVkConfig.cmake
    INSTALL_DESTINATION cmake
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)
write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/cmake/gbVkVersion.cmake
    VERSION ${GB_VK_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_file(${PROJECT_SOURCE_DIR}/cmake/gbVkMacros.cmake.in
    ${PROJECT_BINARY_DIR}/cmake/gbVkMacros.cmake
    COPYONLY
)

# installation - build tree specific package config files
export(EXPORT gbVkTargets FILE ${PROJECT_BINARY_DIR}/cmake/gbVkTargets.cmake NAMESPACE ${GB_LIBRARY_NAMESPACE})
export(PACKAGE gbVk)

# installation - relocatable package config files
install(EXPORT gbVkTargets
    FILE gbVkTargets.cmake
    NAMESPACE ${GB_LIBRARY_NAMESPACE}
    DESTINATION cmake
)
install(FILES
    ${PROJECT_BINARY_DIR}/cmake/gbVkConfig.cmake
    ${PROJECT_BINARY_DIR}/cmake/gbVkMacros.cmake
    ${PROJECT_BINARY_DIR}/cmake/gbVkVersion.cmake
    DESTINATION cmake
)
