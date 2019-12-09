#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_OBJ_PARSER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_OBJ_PARSER_HPP

/** @file
 *
 * @brief OBJ File Parser.
 * @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
 */
#include <gbGraphics/config.hpp>

#include <gbMath/Vector2.hpp>
#include <gbMath/Vector3.hpp>

#include <string>
#include <vector>

namespace GHULBUS_GRAPHICS_NAMESPACE {
/** Parser for Alias OBJ Files.
 * @todo The following features are currently unsupported:
 *       - Offset-based indexing unsupported (entry: 'f -1 -2 -3')
 *       - Multiple face group assignments (entry: 'g group1 group2')
 *       - Non-2d texture coordinates
 *       - Support for smoothing groups
 */
class ObjParser {
public:
    /** Storage for mesh vertex data.
     * @note Position in vector corresponds to (index-1).
     */
    struct VertexData {
        std::vector<GhulbusMath::Point3f> vertex;       ///< Geometry data
        std::vector<GhulbusMath::Normal3f> normal;      ///< Normal data
        std::vector<GhulbusMath::Vector2f> texCoord;    ///< Texture Coordinate
    };

    using IndexType = int32_t;

    /** Storage for mesh face index data.
     * @note Indices are 1-based if positive and offset-based if negative.
     *       The corresponding vertex lies at (index-1) in the respective
     *       VertexData list.
     * @attention Currently, offset-based indices are not supported.
     */
    struct FaceData {
        std::vector<IndexType> faceVertex;              ///< Geometry indices
        std::vector<IndexType> faceNormal;              ///< Normal indices
        std::vector<IndexType> faceTexCoord;            ///< Texture coordinate indices
        int  verticesPerFace;                           ///< Number of vertices per face
        bool hasNormal;                                 ///< true iff Normal indices are available
        bool hasTexCoord;                               ///< true iff Texture coordinate indices are available
    public:
        /** Constructor.
         */
        FaceData();
    };

    /** Type for storing grouped FaceData.
     */
    using FaceGroups = std::vector<FaceData>;

    /** Type for storing names for FaceGroups.
     * @note Each entry holds the name for the FaceData at the same index in FaceGroups.
     */
    using FaceGroupNames = std::vector<std::string>;
    
    /** Data structure for flat vertices.
     * In OBJ, the different vertex attributes (position, normal, texcoord) are stored in independent arrays.
     * The ObjParser flattens these into a single array-of-structures layout.
     */
    struct VertexEntryFlat {
        GhulbusMath::Point3f vertex;                ///< Geometry
        GhulbusMath::Normal3f normal;               ///< Normal
        GhulbusMath::Vector2f texCoord;             ///< Texture Coordinate
    };

    /** Container for flat vertex data.
     */
    using VertexDataFlat = std::vector<VertexEntryFlat>;

    /** Container for flat index data.
     */
    using IndexDataFlat = std::vector<IndexType>;
private:
    VertexData     m_vertexData;            ///< Vertex data
    FaceGroups     m_faceGroups;            ///< Grouped Index data
    FaceGroupNames m_faceGroupNames;        ///< Group Names
    VertexDataFlat m_vertexDataFlat;        ///< Flattened vertex data
    IndexDataFlat  m_indexDataFlat;         ///< Flattened index data
public:
    /** Constructor.
     */
    ObjParser();

    /** Destructor.
     */
    ~ObjParser();

    ObjParser(ObjParser const&) = delete;
    ObjParser& operator=(ObjParser const&) = delete;

    /** Read in data from OBJ file.
     * @param[in] filename Full path to OBJ file.
     */
    void readFile(char const* filename);

    /** Check whether a face group has texture coordinates.
     * @param[in] i Index of the face group.
     * @return True iff the face group has texture coordinates.
     */
    bool groupHasTexCoord(IndexType i) const;

    /** Check whether a face group has per-vertex normal data.
     * @param[in] i Index of the face group.
     * @return True iff the face group has per-vertex normals.
     */
    bool groupHasNormal(IndexType i) const;

    /** Get the number of vertices per face in a face group.
     * @param[in] i Index of the face group.
     * @return Number of vertices per face in the group.
     */
    int groupVerticesPerFace(IndexType i) const;

    /** Get the total number of face groups in the mesh.
     * @return Number of face groups.
     */
    IndexType numberOfGroups() const;

    /** Get the total number of vertices in the mesh.
     * @return Number of vertices.
     */
    IndexType numberOfVertices() const;

    /** Get the total number of texture vertices in the mesh.
     * @return Number of texture coordinate vertices.
     */
    IndexType numberOfTextureVertices() const;

    /** Get the total number of normals in the mesh.
     * @return Number of normals.
     */
    IndexType numberOfNormals() const;

    /** Get the number of faces in a face group.
     * @param[in] i Index of the face group.
     * @return Number of faces in the group.
     */
    IndexType numberOfFacesInGroup(IndexType i) const;

    /** Get the total number of faces.
     * @return Sum of the number of faces in all groups.
     */
    IndexType numberOfFacesTotal() const;

    /** Get the total number of flat faces.
     * @return Number of flattened faces.
     */
    IndexType numberOfFlatFaces() const;

    /** Get the total number of flat vertices.
     * @return Number of flattened vertices.
     */
    IndexType numberOfFlatVertices() const;

    /** Get flat vertex data.
     * @return Flat vertex data.
     */
    VertexDataFlat const& getFlatVertices() const;

    /** Get Flat index data.
     * @return Flat indices (each 3 indices form a face).
     * @note Flat faces are always triangles.
     */
    IndexDataFlat const& getFlatIndices() const;

    /** Get the name of a face group.
     * @param[in] i Index of the face group.
     * @return Name of the face group.
     */
    char const* getGroupName(IndexType i) const;

private:
    /** Clear all internal mesh data.
     */
    void clearMesh();
};
}
#endif
