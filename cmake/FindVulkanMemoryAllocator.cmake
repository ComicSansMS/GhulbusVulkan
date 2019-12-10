
find_path(VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR NAMES vk_mem_alloc.h
    HINTS ${VULKAN_MEMORY_ALLOCATOR_ROOT}
          ${PROJECT_BINARY_DIR}/dependencies/vulkan-memory-allocator/install
    PATH_SUFFIXES src
)
mark_as_advanced(VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VulkanMemoryAllocator REQUIRED_VARS VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR)

if(VulkanMemoryAllocator_FOUND)
    if(NOT TARGET VulkanMemoryAllocator::VulkanMemoryAllocator)
        add_library(VulkanMemoryAllocator::VulkanMemoryAllocator INTERFACE IMPORTED)
        target_include_directories(VulkanMemoryAllocator::VulkanMemoryAllocator INTERFACE
            ${VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR})
        #set_target_properties(VulkanMemoryAllocator::VulkanMemoryAllocator PROPERTIES
        #  INTERFACE_INCLUDE_DIRECTORIES ${VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR})

        if(MSVC)
            if(EXISTS ${VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR}/vk_mem_alloc.natvis)
                target_sources(VulkanMemoryAllocator::VulkanMemoryAllocator INTERFACE
                    ${VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR}/vk_mem_alloc.natvis)
            endif()
        endif()
    endif()
endif()
