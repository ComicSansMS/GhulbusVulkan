#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SPIRV_CODE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SPIRV_CODE_HPP

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

class SpirvCode {
    friend SpirvCode load(std::byte const*, uint32_t);
public:
    struct SpirvVersion {
        uint8_t major;
        uint8_t minor;
    };
private:
    std::unique_ptr<uint32_t[]> m_data;
    uint32_t m_size;
public:
    SpirvCode() = default;
    ~SpirvCode() = default;
    SpirvCode(SpirvCode const&) = delete;
    SpirvCode& operator=(SpirvCode const&) = delete;
    SpirvCode(SpirvCode&&) = default;
    SpirvCode& operator=(SpirvCode&&) = default;

    size_t getSize() const;
    uint32_t const* getCode() const;
    std::vector<uint32_t> getCodeAsStdVector() const;
    SpirvVersion getSpirvVersion() const;
    uint32_t getBound() const;

    static SpirvCode load(std::filesystem::path const& spirv_file);
    static SpirvCode load(std::istream& is);
    static SpirvCode load(std::byte const* data, uint32_t data_size);
};

}
#endif
