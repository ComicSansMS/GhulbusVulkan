cmake_minimum_required(VERSION 3.9)

project(GhulbusVulkan)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

if(MSVC)
    option(GB_PARALLEL_BUILD "If set, uses multiple build processes in parallel." ON)
    if(GB_PARALLEL_BUILD)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif()
endif()

###############################################################################
## Library gbVk
###############################################################################

set(GB_VK_VERSION_MAJOR 0)
set(GB_VK_VERSION_MINOR 1)
set(GB_VK_VERSION_PATCH 0)
set(GB_VK_VERSION "${GB_VK_VERSION_MAJOR}.${GB_VK_VERSION_MINOR}.${GB_VK_VERSION_PATCH}")

option(BUILD_SHARED_LIBS "If checked, generate dynamic instead of static libraries." OFF)

#set(Boost_USE_STATIC_LIBS ON)
#find_package(Boost REQUIRED)

find_package(Vulkan REQUIRED)

find_path(GLFW3_ROOT NAMES lib/cmake/glfw3/glfw3Config.cmake
          HINTS ENV GLFW3_ROOT)
if(GLFW3_ROOT)
    list(APPEND CMAKE_PREFIX_PATH ${GLFW3_ROOT})
endif()
find_package(glfw3 REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(GHULBUS_BASE_ROOT "" CACHE PATH "GhulbusBase installation root directory.")
if(GHULBUS_BASE_ROOT)
    list(APPEND CMAKE_PREFIX_PATH ${GHULBUS_BASE_ROOT}/cmake)
endif()
find_package(gbBase REQUIRED)
if(NOT ((GB_BASE_IS_SHARED AND BUILD_SHARED_LIBS) OR ((NOT GB_BASE_IS_SHARED) AND (NOT BUILD_SHARED_LIBS))))
    message(STATUS "gbBase is shared " ${GB_BASE_IS_SHARED} "; gbVk is shared " ${BUILD_SHARED_LIBS} ".")
    message(FATAL_ERROR "Mixing shared and static builds of Ghulbus libraries is not supported.")
endif()

set(GB_VK_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(GB_VK_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(GB_VK_TEST_DIR ${PROJECT_SOURCE_DIR}/test)

set(GB_VK_SOURCE_FILES
    ${GB_VK_SOURCE_DIR}/CommandBuffer.cpp
    ${GB_VK_SOURCE_DIR}/CommandPool.cpp
    ${GB_VK_SOURCE_DIR}/Device.cpp
    ${GB_VK_SOURCE_DIR}/DeviceMemory.cpp
    ${GB_VK_SOURCE_DIR}/Instance.cpp
    ${GB_VK_SOURCE_DIR}/PhysicalDevice.cpp
    ${GB_VK_SOURCE_DIR}/StringConverters.cpp
    ${GB_VK_SOURCE_DIR}/Swapchain.cpp
)

set(GB_VK_HEADER_FILES
    ${GB_VK_INCLUDE_DIR}/gbVk/CommandBuffer.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/CommandPool.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/config.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Device.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/DeviceMemory.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Exceptions.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/Instance.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/PhysicalDevice.hpp
    ${GB_VK_INCLUDE_DIR}/gbVk/StringConverters.hpp
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
#target_include_directories(gbVk PUBLIC $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>)
#target_compile_definitions(gbVk PUBLIC BOOST_ALL_NO_LIB)
target_link_libraries(gbVk PUBLIC Threads::Threads)
target_link_libraries(gbVk PUBLIC Ghulbus::gbBase)
target_link_libraries(gbVk PUBLIC Vulkan::Vulkan)

if(NOT GB_GENERATE_COVERAGE_INFO)
    target_compile_definitions(gbVk PRIVATE $<$<CONFIG:Debug>:GHULBUS_CONFIG_ASSERT_LEVEL_DEBUG>)
endif()
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
## Tools
###############################################################################
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
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
                GIT_TAG 355ab78f4a89736171d34d6ce271aa36a0ce155d)
        elseif(WIN32)
            set(GB_FETCH_EXTERNAL_CATCH
                URL https://github.com/philsquared/Catch/archive/v2.0.0-develop.5.zip
                URL_HASH MD5=175799e54ca5fe21f1b16a65b032797f)
        else()
            set(GB_FETCH_EXTERNAL_CATCH
                URL https://github.com/philsquared/Catch/archive/v2.0.0-develop.5.tar.gz
                URL_HASH MD5=58a07e5cee760af0c03fea12dfabe40f)
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

add_executable(demo ${PROJECT_SOURCE_DIR}/demo/demo.cpp)
target_link_libraries(demo PUBLIC gbVk glfw)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT demo)

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
