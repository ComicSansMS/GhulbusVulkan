#include <gbGraphics/Mesh.hpp>

#include <gbGraphics/GraphicsInstance.hpp>
#include <gbGraphics/ImageLoader.hpp>
#include <gbGraphics/ObjParser.hpp>

#include <gbVk/Queue.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

Mesh::Mesh(GraphicsInstance& instance, ObjParser const& obj, ImageLoader const& texture_loader)
    :m_vertexBuffer(instance,  obj.numberOfFlatVertices() * sizeof(ObjParser::VertexEntryFlat),
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryUsage::GpuOnly),
    m_indexBuffer(instance, obj.numberOfFlatFaces() * 3 * sizeof(ObjParser::IndexType),
                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryUsage::GpuOnly),
    m_texture(instance, texture_loader.getWidth(), texture_loader.getHeight())
{
    GhulbusVulkan::Queue& transfer_queue = instance.getTransferQueue();
    transfer_queue.stageSubmission(
        m_vertexBuffer.setDataAsynchronously(reinterpret_cast<std::byte const*>(obj.getFlatVertices().data()),
                                             instance.getGraphicsQueueFamilyIndex()));
    transfer_queue.stageSubmission(
        m_indexBuffer.setDataAsynchronously(reinterpret_cast<std::byte const*>(obj.getFlatIndices().data()),
                                            instance.getGraphicsQueueFamilyIndex()));
    transfer_queue.stageSubmission(
        m_texture.setDataAsynchronously(reinterpret_cast<std::byte const*>(texture_loader.getData()),
                                        instance.getGraphicsQueueFamilyIndex()));
}

Mesh::Mesh(GraphicsInstance& instance, VertexData const* vertex_data, size_t n_vertices,
           IndexData const* index_data, size_t n_indices, ImageLoader const& texture_loader)
    :m_vertexBuffer(instance,  n_vertices * sizeof(VertexData),
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryUsage::GpuOnly),
    m_indexBuffer(instance, n_indices * sizeof(ObjParser::IndexType),
                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryUsage::GpuOnly),
    m_texture(instance, texture_loader.getWidth(), texture_loader.getHeight())
{
    GhulbusVulkan::Queue& transfer_queue = instance.getTransferQueue();
    transfer_queue.stageSubmission(
        m_vertexBuffer.setDataAsynchronously(reinterpret_cast<std::byte const*>(vertex_data),
                                             instance.getGraphicsQueueFamilyIndex()));
    transfer_queue.stageSubmission(
        m_indexBuffer.setDataAsynchronously(reinterpret_cast<std::byte const*>(index_data),
                                            instance.getGraphicsQueueFamilyIndex()));
    transfer_queue.stageSubmission(
        m_texture.setDataAsynchronously(reinterpret_cast<std::byte const*>(texture_loader.getData()),
                                        instance.getGraphicsQueueFamilyIndex()));
}

uint32_t Mesh::getNumberOfIndices() const
{
    return static_cast<uint32_t>(m_indexBuffer.getSize() / sizeof(IndexData));
}

uint32_t Mesh::getNumberOfVertices() const
{
    return static_cast<uint32_t>(m_vertexBuffer.getSize() / sizeof(VertexData));
}

GhulbusGraphics::MemoryBuffer& Mesh::getVertexBuffer()
{
    return m_vertexBuffer;
}

GhulbusGraphics::MemoryBuffer& Mesh::getIndexBuffer()
{
    return m_indexBuffer;
}

GhulbusGraphics::Image2d& Mesh::getTexture()
{
    return m_texture;
}
}