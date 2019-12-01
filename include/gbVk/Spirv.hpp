#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SPIRV_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SPIRV_HPP

/** @file
*
* @brief SPIR-V loader and code representation.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <filesystem>
#include <memory>

namespace GHULBUS_VULKAN_NAMESPACE
{
namespace Spirv
{
class Code {
    friend Code load(std::byte const*, uint32_t);
public:
    struct SpirvVersion {
        uint8_t major;
        uint8_t minor;
    };
private:
    std::unique_ptr<uint32_t[]> m_data;
    uint32_t m_size;
public:
    Code() = default;
    ~Code() = default;
    Code(Code const&) = delete;
    Code& operator=(Code const&) = delete;
    Code(Code&&) = default;
    Code& operator=(Code&&) = default;

    size_t getSize() const;
    uint32_t const* getCode() const;
    SpirvVersion getSpirvVersion() const;
    uint32_t getBound() const;
};

Code load(std::filesystem::path const& spirv_file);
Code load(std::istream& is);
Code load(std::byte const* data, uint32_t data_size);
}
}
#endif
