#include <gbVk/SpirvCode.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

#include <cstring>
#include <fstream>
#include <limits>
#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
size_t SpirvCode::getSize() const
{
    return m_size;
}

uint32_t const* SpirvCode::getCode() const
{
    return m_data.get();
}

SpirvCode::SpirvVersion SpirvCode::getSpirvVersion() const
{
    SpirvVersion ret;
    ret.major = ((m_data[1] >> 16) & 0xff);
    ret.minor = ((m_data[1] >> 8) & 0xff);
    return ret;
}

uint32_t SpirvCode::getBound() const
{
    return m_data[3];
}

SpirvCode SpirvCode::load(std::filesystem::path const& spirv_file)
{
    std::ifstream fin(spirv_file, std::ios_base::binary);
    if(!fin) {
        GHULBUS_THROW(Exceptions::IOError() << Ghulbus::Exception_Info::filename(spirv_file.string()),
                      "Unable to open file for reading.");
    }
    return load(fin);
}

SpirvCode SpirvCode::load(std::istream& is)
{
    GHULBUS_PRECONDITION(is);
    auto start_pos = is.tellg();
    is.seekg(0, std::ios_base::end);
    auto eof_pos = is.tellg();
    is.seekg(start_pos);
    auto const buffer_size = eof_pos - start_pos;
    if(buffer_size > std::numeric_limits<uint32_t>::max()) {
        GHULBUS_THROW(Exceptions::IOError(), "Maximum buffer size exceeded.");
    }
    std::vector<std::byte> buffer;
    buffer.resize(buffer_size);
    is.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    if(!is) {
        GHULBUS_THROW(Exceptions::IOError(), "Error reading SPIR-V input stream.");
    }
    return load(buffer.data(), static_cast<uint32_t>(buffer_size));
}

SpirvCode SpirvCode::load(std::byte const* data, uint32_t data_size)
{
    GHULBUS_PRECONDITION(data_size % sizeof(uint32_t) == 0);
    SpirvCode ret;
    uint32_t const spirv_magic_number = 0x07230203;
    uint32_t const spirv_magic_number_swapped = 0x03022307;
    uint32_t check_magic_number;
    std::memcpy(&check_magic_number, data, sizeof(uint32_t));
    bool swap_endianess = [=]() {
        if(check_magic_number == spirv_magic_number) {
            return false;
        } else if(check_magic_number == spirv_magic_number_swapped) {
            return true;
        } else {
            GHULBUS_THROW(Exceptions::IOError(), "Data is not a valid SPIR-V stream.");
        }
    }();

    size_t const n_elements = data_size / sizeof(uint32_t);
    if(swap_endianess) {
        ret.m_data.reset(new uint32_t[n_elements]);
        ret.m_size = data_size;
        for(size_t i = 0; i < n_elements; ++i) {
            auto const base_idx = i * sizeof(uint32_t);
            uint32_t const tmp = (std::to_integer<uint32_t>(data[base_idx + 3])) |
                                 (std::to_integer<uint32_t>(data[base_idx + 2]) << 8) |
                                 (std::to_integer<uint32_t>(data[base_idx + 1]) << 16) |
                                 (std::to_integer<uint32_t>(data[base_idx]) << 24);
            ret.m_data[i] = tmp;
        }
    } else {
        ret.m_data.reset(new uint32_t[n_elements]);
        ret.m_size = data_size;
        std::memcpy(ret.m_data.get(), data, data_size);
    }

    return ret;
}
}
