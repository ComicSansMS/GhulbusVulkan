#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_MESH_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_MESH_HPP

/** @file
*
* @brief Triangle Mesh.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/GraphicsInstance.hpp>
#include <gbGraphics/Image2d.hpp>
#include <gbGraphics/ImageLoader.hpp>
#include <gbGraphics/IndexData.hpp>
#include <gbGraphics/MemoryBuffer.hpp>
#include <gbGraphics/ObjParser.hpp>
#include <gbGraphics/VertexData.hpp>

#include <gbVk/Queue.hpp>

#include <gbMath/Vector2.hpp>
#include <gbMath/Vector3.hpp>

#include <type_traits>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;
class ImageLoader;
class ObjParser;

template<typename VertexData_T = VertexData<
        VertexComponent<GhulbusMath::Point3f, VertexComponentSemantics::Position>,
        VertexComponent<GhulbusMath::Normal3f, VertexComponentSemantics::Normal>,
        VertexComponent<GhulbusMath::Vector2f, VertexComponentSemantics::Texture>>,
    typename IndexData_T = IndexData<IndexFormatBase::PrimitiveTopology::TriangleList, uint32_t>>
class Mesh {
public:
    using VertexData = VertexData_T;
    using VertexFormat = typename VertexData::Format;
    using IndexData = IndexData_T;
private:
    GhulbusGraphics::MemoryBuffer m_vertexBuffer;
    GhulbusGraphics::MemoryBuffer m_indexBuffer;
    GhulbusGraphics::Image2d m_texture;
public:
    Mesh(GraphicsInstance& instance, ObjParser const& obj, ImageLoader const& texture_loader);
    Mesh(GraphicsInstance& instance, VertexData const& vertex_data,
         IndexData const& index_data, ImageLoader const& texture_loader);

    uint32_t getNumberOfIndices() const;
    uint32_t getNumberOfVertices() const;

    GhulbusGraphics::MemoryBuffer& getVertexBuffer();
    GhulbusGraphics::MemoryBuffer& getIndexBuffer();
    GhulbusGraphics::Image2d& getTexture();
};

template<typename V_T, typename I_T>
inline Mesh<V_T, I_T>::Mesh(GraphicsInstance& instance, ObjParser const& obj, ImageLoader const& texture_loader)
    : m_vertexBuffer(instance,  obj.numberOfFlatVertices() * sizeof(ObjParser::VertexEntryFlat),
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

template<typename V_T, typename I_T>
inline Mesh<V_T, I_T>::Mesh(GraphicsInstance& instance, VertexData const& vertex_data,
                            IndexData const& index_data, ImageLoader const& texture_loader)
    :m_vertexBuffer(instance, vertex_data.size() * sizeof(VertexData::Storage),
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryUsage::GpuOnly),
    m_indexBuffer(instance, index_data.size() * sizeof(IndexData::IndexType),
                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryUsage::GpuOnly),
    m_texture(instance, texture_loader.getWidth(), texture_loader.getHeight())
{
    GhulbusVulkan::Queue& transfer_queue = instance.getTransferQueue();
    transfer_queue.stageSubmission(
        m_vertexBuffer.setDataAsynchronously(vertex_data.data(),
                                             instance.getGraphicsQueueFamilyIndex()));
    transfer_queue.stageSubmission(
        m_indexBuffer.setDataAsynchronously(index_data.data(),
                                            instance.getGraphicsQueueFamilyIndex()));
    transfer_queue.stageSubmission(
        m_texture.setDataAsynchronously(reinterpret_cast<std::byte const*>(texture_loader.getData()),
                                        instance.getGraphicsQueueFamilyIndex()));
}

template<typename VertexData_T, typename IndexData_T>
inline uint32_t Mesh<VertexData_T, IndexData_T>::getNumberOfIndices() const
{
    return static_cast<uint32_t>(m_indexBuffer.getSize() / sizeof(IndexData::IndexType::ValueType));
}

template<typename VertexData_T, typename IndexData_T>
inline uint32_t Mesh<VertexData_T, IndexData_T>::getNumberOfVertices() const
{
    return static_cast<uint32_t>(m_vertexBuffer.getSize() / sizeof(VertexData::Storage));
}

template<typename VertexData_T, typename IndexData_T>
inline GhulbusGraphics::MemoryBuffer& Mesh<VertexData_T, IndexData_T>::getVertexBuffer()
{
    return m_vertexBuffer;
}

template<typename VertexData_T, typename IndexData_T>
inline GhulbusGraphics::MemoryBuffer& Mesh<VertexData_T, IndexData_T>::getIndexBuffer()
{
    return m_indexBuffer;
}

template<typename VertexData_T, typename IndexData_T>
inline GhulbusGraphics::Image2d& Mesh<VertexData_T, IndexData_T>::getTexture()
{
    return m_texture;
}


class AnyMesh {
private:
    using Storage = std::byte alignas(Mesh<>)[sizeof(Mesh<>)];

    class MeshConcept {
    public:
        virtual ~MeshConcept() = default;
        virtual MeshConcept* moveInto(Storage*) = 0;
        virtual uint32_t getNumberOfIndices() const = 0;
        virtual uint32_t getNumberOfVertices() const = 0;
        virtual GhulbusGraphics::MemoryBuffer& getVertexBuffer() = 0;
        virtual GhulbusGraphics::MemoryBuffer& getIndexBuffer() = 0;
        virtual GhulbusGraphics::Image2d& getTexture() = 0;
    };
    template<typename VertexData_T, typename IndexData_T>
    class MeshModel : public MeshConcept {
    private:
        static_assert(sizeof(Mesh<VertexData_T, IndexData_T>) == sizeof(Mesh<>),
                      "All Mesh types must have same size.");
        Mesh<VertexData_T, IndexData_T> m_mesh;
    public:
        MeshModel(Mesh<VertexData_T, IndexData_T>&& mesh)
            :m_mesh(std::move(mesh))
        {}
        ~MeshModel() override = default;

        MeshConcept* moveInto(Storage* s) override {
            return new (s) MeshModel<VertexData_T, IndexData_T>(std::move(m_mesh));
        }
        uint32_t getNumberOfIndices() const override {
            return m_mesh.getNumberOfIndices();
        }
        uint32_t getNumberOfVertices() const override {
            return m_mesh.getNumberOfVertices();
        }
        GhulbusGraphics::MemoryBuffer& getVertexBuffer() override {
            return m_mesh.getVertexBuffer();
        }
        GhulbusGraphics::MemoryBuffer& getIndexBuffer() override {
            return m_mesh.getIndexBuffer();
        }
        GhulbusGraphics::Image2d& getTexture() override {
            return m_mesh.getTexture();
        }
    };

    Storage m_storage;
    MeshConcept* m_mesh;
public:
    AnyMesh()
        :m_mesh(nullptr)
    {}

    template<typename VertexData_T, typename IndexData_T>
    AnyMesh(Mesh<VertexData_T, IndexData_T>&& mesh)
        :m_mesh(new(&m_storage) MeshModel<VertexData_T, IndexData_T>(std::move(mesh)))
    {}

    AnyMesh(AnyMesh&& rhs) noexcept
        :m_mesh(rhs.m_mesh->moveInto(&m_storage))
    {
        rhs.m_mesh = nullptr;
    }

    ~AnyMesh()
    {
        if (m_mesh) { m_mesh->~MeshConcept(); };
    }

    AnyMesh& operator=(AnyMesh&& rhs) noexcept {
        if (&rhs != this) {
            m_mesh->~MeshConcept();
            m_mesh = rhs.m_mesh->moveInto(&m_storage);
            rhs.m_mesh = nullptr;
        }
        return *this;
    }

    operator bool() const {
        return m_mesh != nullptr;
    }

    bool operator!() const {
        return m_mesh == nullptr;
    }

    bool isEmpty() const {
        return m_mesh == nullptr;
    }

    uint32_t getNumberOfIndices() const {
        return m_mesh->getNumberOfIndices();
    }
    uint32_t getNumberOfVertices() const {
        return m_mesh->getNumberOfVertices();
    }

    GhulbusGraphics::MemoryBuffer& getVertexBuffer() {
        return m_mesh->getVertexBuffer();
    }

    GhulbusGraphics::MemoryBuffer& getIndexBuffer() {
        return m_mesh->getIndexBuffer();
    }

    GhulbusGraphics::Image2d& getTexture() {
        return m_mesh->getTexture();
    }
};
}
#endif
