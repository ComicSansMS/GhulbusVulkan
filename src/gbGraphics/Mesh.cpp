#include <gbGraphics/Mesh.hpp>

#include <gbGraphics/GraphicsInstance.hpp>
#include <gbGraphics/ObjParser.hpp>

#include <gbVk/Queue.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

Mesh::Mesh(GraphicsInstance& instance, ObjParser const& obj)
    :m_vertexBuffer(instance,  obj.numberOfFlatVertices() * sizeof(ObjParser::VertexEntryFlat),
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryUsage::GpuOnly),
    m_indexBuffer(instance, obj.numberOfFlatFaces() * sizeof(ObjParser::IndexType),
                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryUsage::GpuOnly)
{
    GhulbusVulkan::Queue& transfer_queue = instance.getTransferQueue();
    transfer_queue.stageSubmission(
        m_vertexBuffer.setDataAsynchronously(reinterpret_cast<std::byte const*>(obj.getFlatVertices().data()),
                                             instance.getGraphicsQueueFamilyIndex()));
    transfer_queue.stageSubmission(
        m_indexBuffer.setDataAsynchronously(reinterpret_cast<std::byte const*>(obj.getFlatIndices().data()),
                                            instance.getGraphicsQueueFamilyIndex()));
}

Mesh::~Mesh() = default;

GhulbusGraphics::MemoryBuffer& Mesh::getVertexBuffer()
{
    return m_vertexBuffer;
}

GhulbusGraphics::MemoryBuffer& Mesh::getIndexBuffer()
{
    return m_indexBuffer;
}
}
