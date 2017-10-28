
#include <gbVk/StringConverters.hpp>

#include <ostream>

namespace GHULBUS_VULKAN_NAMESPACE
{

char const* to_string(VkResult r)
{
#define GHULBUS_VULKAN_VKRESULT_PRINT_CASE(ec) case ec: return #ec
    switch(r) {
    default: return "Unknown";
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_SUCCESS);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_NOT_READY);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_TIMEOUT);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_EVENT_SET);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_EVENT_RESET);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_INCOMPLETE);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_OUT_OF_HOST_MEMORY);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_INITIALIZATION_FAILED);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_DEVICE_LOST);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_MEMORY_MAP_FAILED);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_LAYER_NOT_PRESENT);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_EXTENSION_NOT_PRESENT);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_FEATURE_NOT_PRESENT);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_INCOMPATIBLE_DRIVER);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_TOO_MANY_OBJECTS);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_FORMAT_NOT_SUPPORTED);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_FRAGMENTED_POOL);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_SURFACE_LOST_KHR);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_SUBOPTIMAL_KHR);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_OUT_OF_DATE_KHR);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_VALIDATION_FAILED_EXT);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_INVALID_SHADER_NV);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_OUT_OF_POOL_MEMORY_KHR);
        GHULBUS_VULKAN_VKRESULT_PRINT_CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR);

    }
#undef GHULBUS_VULKAN_VKRESULT_PRINT_CASE
}

char const* to_string(VkPhysicalDeviceType t)
{
    switch(t)
    {
    default:                                     return "Unknown";
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:          return "Other";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return "Discrete GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    return "Virtual GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:            return "CPU";
    }
}

std::string to_string(VkQueueFlags flags)
{
    std::string ret;
    if(flags & VK_QUEUE_GRAPHICS_BIT) {
        ret += "Graphics ";
    }
    if(flags & VK_QUEUE_COMPUTE_BIT) {
        ret += "Compute ";
    }
    if(flags & VK_QUEUE_TRANSFER_BIT) {
        ret += "Transfer ";
    }
    if(flags & VK_QUEUE_SPARSE_BINDING_BIT) {
        ret += "Sparse_Binding ";
    }
    if(!ret.empty()) { ret.pop_back(); }
    return ret;
}

std::string memory_type_properties_to_string(uint32_t propertyFlags)
{
    std::string ret;
    if(propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
        ret += "Device_Local ";
    }
    if(propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        ret += "Host_Visible ";
    }
    if(propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        ret += "Host_Coherent ";
    }
    if(propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
        ret += "Host_Cached ";
    }
    if(propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
        ret += "Lazy ";
    }
    if(!ret.empty()) { ret.pop_back(); }
    return ret;
}

std::string memory_heap_properties_to_string(uint32_t propertyFlags)
{
    std::string ret;
    if(propertyFlags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        ret += "Device_Local ";
    }
    if(!ret.empty()) { ret.pop_back(); }
    return ret;
}

std::string memory_size_to_string(VkDeviceSize size)
{
    if(size > (1 << 30)) {
        return std::to_string(size >> 30u) + " GB";
    }
    else if(size > (1 << 20)) {
        return std::to_string(size >> 20u) + " MB";
    }
    else if(size > (1 << 10)) {
        return std::to_string(size >> 10u) + " KB";
    }
    return std::to_string(size) + " byte" + ((size == 1) ? "" : "s");
}

std::string version_to_string(uint32_t v)
{
    return std::to_string(VK_VERSION_MAJOR(v)) + "." +
           std::to_string(VK_VERSION_MINOR(v)) + "." +
           std::to_string(VK_VERSION_PATCH(v));
}

std::string uuid_to_string(uint8_t uuid[VK_UUID_SIZE])
{
    std::string res;
    res.reserve(36);

    auto to_char = [](int i) -> char {
           return (i <= 9) ? static_cast<char>('0' + i) : static_cast<char>('a' + (i-10));
        };

    for(int i = 0; i < VK_UUID_SIZE; ++i) {
        auto const hi = (uuid[i] >> 4) & 0x0F;
        auto const lo = uuid[i] & 0x0F;
        res.push_back(to_char(hi));
        res.push_back(to_char(lo));

        if (i == 3 || i == 5 || i == 7 || i == 9) {
            res.push_back('-');
        }
    }
    return res;
}
}

std::ostream& operator<<(std::ostream& os, VkResult r)
{
    os << GHULBUS_VULKAN_NAMESPACE::to_string(r) << " (" << static_cast<int>(r) << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, VkPhysicalDeviceType t)
{
    return os << GHULBUS_VULKAN_NAMESPACE::to_string(t);
}

std::ostream& operator<<(std::ostream& os, VkLayerProperties const& lp)
{
    os << lp.layerName << '\n'
       << "  Spec V-" << VK_VERSION_MAJOR(lp.specVersion) << "."
                      << VK_VERSION_MINOR(lp.specVersion) << "."
                      << VK_VERSION_PATCH(lp.specVersion)
       << " Impl V-" << VK_VERSION_MAJOR(lp.implementationVersion) << "."
                     << VK_VERSION_MINOR(lp.implementationVersion) << "."
                     << VK_VERSION_PATCH(lp.implementationVersion) << "\n  "
       << lp.description;
    return os;
}

std::ostream& operator<<(std::ostream& os, VkExtensionProperties const& ep)
{
    os << ep.extensionName << " V-" << VK_VERSION_MAJOR(ep.specVersion) << "."
                                    << VK_VERSION_MINOR(ep.specVersion) << "."
                                    << VK_VERSION_PATCH(ep.specVersion);
    return os;
}
