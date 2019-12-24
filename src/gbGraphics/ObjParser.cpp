#include <gbGraphics/ObjParser.hpp>

#include <gbGraphics/Exceptions.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>

#include <algorithm>
#include <charconv>
#include <fstream>
#include <numeric>
#include <limits>
#include <regex>
#include <tuple>
#include <unordered_map>

#if WIN32
#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#   define NOMINMAX
#endif
#include <Windows.h>
#endif

namespace GHULBUS_GRAPHICS_NAMESPACE {
namespace {

/** Index Tuple type used for face flattening
 */
struct IndexTuple {
    using IndexType = ObjParser::IndexType;
    IndexType vertexIndex;
    IndexType normalIndex;
    IndexType textureIndex;
public:
    IndexTuple()
        : vertexIndex(0), normalIndex(0), textureIndex(0)
    {}
    IndexTuple(IndexType v, IndexType n, IndexType t)
        : vertexIndex(v), normalIndex(n), textureIndex(t)
    {}
};

inline bool operator==(IndexTuple const& lhs, IndexTuple const& rhs) {
    return ((lhs.vertexIndex == rhs.vertexIndex) &&
            (lhs.normalIndex == rhs.normalIndex) &&
            (lhs.textureIndex == rhs.textureIndex));
}

inline bool operator<(IndexTuple const& lhs, IndexTuple const& rhs) {
    return std::tie(lhs.vertexIndex, lhs.normalIndex, lhs.textureIndex) <
           std::tie(rhs.vertexIndex, rhs.normalIndex, rhs.textureIndex);
}

struct IndexTupleHash
{
    inline size_t operator()(IndexTuple const& t) const {
        return (t.vertexIndex) ^ (t.normalIndex << 16) ^ (t.textureIndex << 21);
    }
};

using IndexTupleMap = std::unordered_map<IndexTuple, ObjParser::IndexType, IndexTupleHash>;

using Buffer = std::vector<char>;

void readDataFromFile(char const* filename, Buffer* out_data);
void parseData(Buffer const& data,
               ObjParser::VertexData& out_vertex_data,
               ObjParser::FaceGroups& out_face_groups,
               ObjParser::FaceGroupNames& out_group_names,
               ObjParser::VertexDataFlat& out_flat_vertex_data,
               ObjParser::IndexDataFlat& out_flat_index_data);
}

ObjParser::FaceData::FaceData()
    :verticesPerFace(0), hasNormal(false), hasTexCoord(false)
{
}

ObjParser::ObjParser()
{
}

ObjParser::~ObjParser()
{
}

void ObjParser::readFile(char const* filename)
{
    Buffer data;
    readDataFromFile(filename, &data);
    clearMesh();
    parseData(data, m_vertexData, m_faceGroups, m_faceGroupNames, m_vertexDataFlat, m_indexDataFlat);
}

bool ObjParser::groupHasTexCoord(IndexType i) const
{
    GHULBUS_PRECONDITION((i >= 0) && (i < m_faceGroups.size()));
    return m_faceGroups[i].hasTexCoord;
}

bool ObjParser::groupHasNormal(IndexType i) const
{
    GHULBUS_PRECONDITION((i >= 0) && (i < m_faceGroups.size()));
    return m_faceGroups.at(i).hasNormal;
}

int ObjParser::groupVerticesPerFace(IndexType i) const
{
    GHULBUS_PRECONDITION((i >= 0) && (i < m_faceGroups.size()));
    return m_faceGroups.at(i).verticesPerFace;
}

ObjParser::IndexType ObjParser::numberOfGroups() const
{
    GHULBUS_ASSERT(m_faceGroups.size() < std::numeric_limits<IndexType>::max());
    return static_cast<IndexType>(m_faceGroups.size());
}

ObjParser::IndexType ObjParser::numberOfVertices() const
{
    GHULBUS_ASSERT(m_vertexData.vertex.size() < std::numeric_limits<IndexType>::max());
    return static_cast<IndexType>(m_vertexData.vertex.size());
}

ObjParser::IndexType ObjParser::numberOfTextureVertices() const
{
    GHULBUS_ASSERT(m_vertexData.texCoord.size() < std::numeric_limits<IndexType>::max());
    return static_cast<IndexType>(m_vertexData.texCoord.size());
}

ObjParser::IndexType ObjParser::numberOfNormals() const
{
    GHULBUS_ASSERT(m_vertexData.normal.size() < std::numeric_limits<IndexType>::max());
    return static_cast<IndexType>(m_vertexData.normal.size());
}

ObjParser::IndexType ObjParser::numberOfFacesInGroup(IndexType i) const
{
    GHULBUS_PRECONDITION((i >= 0) && (i < m_faceGroups.size()));
    GHULBUS_ASSERT(m_faceGroups[i].faceVertex.size() < std::numeric_limits<IndexType>::max());
    return static_cast<IndexType>(m_faceGroups[i].faceVertex.size() / m_faceGroups[i].verticesPerFace);
}

ObjParser::IndexType ObjParser::numberOfFacesTotal() const
{
    return std::accumulate(m_faceGroups.begin(), m_faceGroups.end(), 0,
            [](IndexType init, FaceData const& group) -> IndexType
            {
                GHULBUS_ASSERT(group.faceVertex.size() < std::numeric_limits<IndexType>::max() - init);
                return init + static_cast<IndexType>(group.faceVertex.size() / group.verticesPerFace);
            }
        );
}

ObjParser::IndexType ObjParser::numberOfFlatFaces() const
{
    GHULBUS_ASSERT(m_indexDataFlat.size() < std::numeric_limits<IndexType>::max());
    GHULBUS_ASSERT(m_indexDataFlat.size() % 3 == 0);
    return static_cast<IndexType>(m_indexDataFlat.size() / 3);
}

ObjParser::IndexType ObjParser::numberOfFlatVertices() const
{
    GHULBUS_ASSERT(m_vertexDataFlat.size() < std::numeric_limits<IndexType>::max());
    return static_cast<IndexType>(m_vertexDataFlat.size());
}

ObjParser::VertexDataFlat const& ObjParser::getFlatVertices() const
{
    return m_vertexDataFlat;
}

ObjParser::IndexDataFlat const& ObjParser::getFlatIndices() const
{
    return m_indexDataFlat;
}

char const* ObjParser::getGroupName(IndexType i) const
{
    GHULBUS_PRECONDITION((i >= 0) && (i < m_faceGroupNames.size()));
    return m_faceGroupNames[i].c_str();
}

void ObjParser::clearMesh()
{
    m_vertexData.vertex.clear();
    m_vertexData.normal.clear();
    m_vertexData.texCoord.clear();
    m_faceGroups.clear();
    m_faceGroupNames.clear();
    m_vertexDataFlat.getStorage().clear();
    m_indexDataFlat.clear();
}

namespace {
void readDataFromFile(char const* filename, Buffer* out_data)
{
#if WIN32
    HANDLE fin = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, nullptr);
    if (fin == INVALID_HANDLE_VALUE) {
        GHULBUS_THROW(Exceptions::IOError{} << Exception_Info::filename(filename),
                      "Unable to open file.");
    }
    auto guard_file_handle = Ghulbus::finally([fin]() { CloseHandle(fin); });
    LARGE_INTEGER lfilesize;
    if ((GetFileSizeEx(fin, &lfilesize) == 0) || (lfilesize.QuadPart >= std::numeric_limits<DWORD>::max())) {
        GHULBUS_THROW(Exceptions::IOError() << Exception_Info::filename(filename),
                      "File size error.");
    }
    DWORD const filesize = static_cast<DWORD>(lfilesize.QuadPart);
    out_data->clear();
    out_data->resize(filesize);
    if (DWORD read; (ReadFile(fin, out_data->data(), filesize, &read, nullptr) == 0) || (read != filesize)) {
        GHULBUS_THROW(Exceptions::IOError() << Exception_Info::filename(filename),
                      "File read error.");
    }
#else
    //open file
    std::ifstream fin(filename, std::ios_base::binary);
    if (fin.fail()) {
        GHULBUS_THROW(Exceptions::IOError{} << Exception_Info::filename(filename),
                      "Unable to open file.");
    }

    //get file size
    fin.seekg(0, std::ios_base::end);
    size_t filesize = static_cast<size_t>(fin.tellg());
    fin.seekg(0, std::ios_base::beg);
    if ((fin.fail()) || (filesize == 0)) {
        GHULBUS_THROW(Exceptions::IOError() << Exception_Info::filename(filename),
                      "File size error.");
    }
    
    //read file
    out_data->clear();
    out_data->resize(filesize);
    fin.read(reinterpret_cast<char*>(&out_data->front()), filesize);
    if (static_cast<size_t>(fin.gcount()) != filesize) {
        GHULBUS_THROW(Exceptions::IOError() << Exception_Info::filename(filename)
                                            << Exception_Info::io_offset(fin.gcount()),
                      "File read error.");
    }
#endif
}

/** Skip to the end of the current line.
 * @param[in] position Iterator to anywhere in the file.
 * @param[in] eof Iterator to the end of file.
 * @return Iterator to the the next newline or eof.
 */
inline Buffer::const_iterator getEndOfLine(Buffer::const_iterator const& position,
                                           Buffer::const_iterator const& eof)
{
    return std::find(position, eof, '\n');
}

/** Skip whitespace.
 * @param[in] position Iterator to anywhere in the file.
 * @param[in] eof Iterator to the end of file.
 * @param[in,out] line_count Line count will be incremented for every encountered newline.
 * @return Iterator to the the next non-whitespace or eof.
 */
inline Buffer::const_iterator skipWhitespace(Buffer::const_iterator const& position,
                                             Buffer::const_iterator const& eof,
                                             int* line_count=NULL)
{
    Buffer::const_iterator ret = position;
    //skip whitespace
    for (;;) {
        ret = std::find_if_not(ret, eof, [](char c) {
                return (c == '\r') || (c == ' ') || (c == '\t');
            });
        if ((ret == eof) || ((*ret) != '\n')) { break; }
        if (line_count) { ++(*line_count); }
        ++ret;
    }
    return ret;
}

/** Parse a 2d vertex vector.
 * @param[in,out] p Iterator pointing to the first vertex coordinate;
 *                  On return, points to new location to proceed parsing from.
 * @param[out] out std::vector that will receive vertex data.
 */
inline void parseVertex2(Buffer::const_iterator& p, std::vector<GhulbusMath::Vector2f>* out)
{
    float f1, f2;
    char const* pos = &(*p);
    char* pend;
    f1 = std::strtof(pos, &pend);
    pos = pend;
    f2 = std::strtof(pos, &pend);
    out->emplace_back(f1, f2);
    p += pend - &(*p);
}

/** Parse a 3d vertex vector.
 * @param[in,out] p Iterator pointing to the first vertex coordinate;
 *                  On return, points to new location to proceed parsing from.
 * @param[out] out std::vector that will receive vertex data.
 */
template<typename Tag>
inline void parseVertex3(Buffer::const_iterator& p, std::vector<GhulbusMath::Vector3Impl<float, Tag>>* out )
{
    float f1, f2, f3;
    char const* pos = &(*p);
    char* pend;
    f1 = std::strtof(pos, &pend);
    pos = pend;
    f2 = std::strtof(pos, &pend);
    pos = pend;
    f3 = std::strtof(pos, &pend);
    out->emplace_back(f1, f2, f3);
    p += pend - &(*p);
}

/** Determine number of vertices per face and what per-vertex data is available.
 * @param[in] p Iterator to the beginning of a line with face data.
 * @param[in] eof Iterator to eof.
 * @param[out] out_mesh Will receive face layout.
 */
void scanFaceLayout(Buffer::const_iterator const& p,
                    Buffer::const_iterator const& eof,
                    ObjParser::FaceData* const out_mesh)
{
    std::string const line(p, getEndOfLine(p, eof));
    std::string tmp;        //< used to get number of vertices per face
    if (std::regex_search(line, std::regex("(\\s)+(-)?(\\d)+//(-)?(\\d)+(\\s)+"))) {
        // layout: v//vn
        out_mesh->hasNormal = true;
        out_mesh->hasTexCoord = false;
        tmp = std::regex_replace(line, std::regex("(-)?(\\d)+//(-)?(\\d)+"), std::string("V"));
    } else if (std::regex_search(line, std::regex("(\\s)+(-)?(\\d)+/(-)?(\\d)+(\\s)+"))) {
        // layout: v/vt
        out_mesh->hasNormal = false;
        out_mesh->hasTexCoord = true;
        tmp = std::regex_replace(line, std::regex("(-)?(\\d)+/(-)?(\\d)+"), std::string("V"));
    } else if (std::regex_search(line, std::regex("(\\s)+(-)?(\\d)+/(-)?(\\d)+/(-)?(\\d)+(\\s)+"))) {
        // layout: v/vt/vn
        out_mesh->hasNormal = true;
        out_mesh->hasTexCoord = true;
        tmp = std::regex_replace(line, std::regex("(-)?(\\d)+/(-)?(\\d)+/(-)?(\\d)+"), std::string("V"));
    } else {
        // layout: v
        out_mesh->hasNormal = false;
        out_mesh->hasTexCoord = false;
        tmp = std::regex_replace(line, std::regex("(-)?(\\d)+"), std::string("V"));
    }
    //count the 'V's in tmp to get vertices per face:
    out_mesh->verticesPerFace = static_cast<int>(std::count(tmp.begin(), tmp.end(), 'V'));
    if ((out_mesh->verticesPerFace != 3) && (out_mesh->verticesPerFace != 4)) {
        GHULBUS_THROW(Exceptions::IOError{}, "Invalid number of vertices.");
    }
}

/** Parse a vertex entry.
 * @param[in,out] p Iterator to beginning of line; On return, points to new location to proceed parsing from.
 * @param[out] out_mesh Will receive new vertex data.
 */
inline void parseVertex(Buffer::const_iterator& p,
                        ObjParser::VertexData* const out_mesh)
{
    //skip 'v' and save next char
    char c = *(++p);
    //skip next char
    ++p;
    switch(c) {
        case ' ':
            // geometry vertex
            parseVertex3(p, &out_mesh->vertex);
            break;
        case 't':
            // texture vertex
            parseVertex2(p, &out_mesh->texCoord);
            break;
        case 'n':
            // vertex normal
            parseVertex3(p, &out_mesh->normal);
            break;
        default:
            //unknown label
            GHULBUS_LOG(Warning, "OBJ Parser encountered unknown vertex label: \'v" << c << "\'");
            break;
        }
}

/** Parse a face entry line from OBJ.
 * @param[in,out] p Iterator to beginning of line; On return, points to new location to proceed parsing from.
 * @param[out] out_mesh Will receive new face data.
 */
inline void parseFace(Buffer::const_iterator& p,
                      ObjParser::FaceData* const out_mesh,
                      std::vector<IndexTuple>* const out_index_tuple,
                      ObjParser::VertexData const& vertex_data)
{
    using IndexType = ObjParser::IndexType;
    IndexType const max_vertex_index = static_cast<IndexType>(vertex_data.vertex.size()) + 1;
    IndexType const max_texture_index = static_cast<IndexType>(vertex_data.texCoord.size()) + 1;
    IndexType const max_normal_index = static_cast<IndexType>(vertex_data.normal.size()) + 1;
    //skip 'f'
    char const* pos = &(*(++p));
    //parse face indices
    char* pend = NULL;
    for (int i=0; i < out_mesh->verticesPerFace; ++i) {
        //vertex index
        IndexType v = static_cast<IndexType>(std::strtoll(pos, &pend, 0));
        if (v < 0) { v += max_vertex_index; }
        GHULBUS_ASSERT(v >= 0);
        out_mesh->faceVertex.push_back(v);
        (*out_index_tuple)[i].vertexIndex = v;
        //texcoord index (optional)
        if (out_mesh->hasTexCoord) {
            pos = pend + 1;
            v = static_cast<IndexType>(std::strtoll(pos, &pend, 0));
            if (v < 0) { v += max_texture_index; }
            GHULBUS_ASSERT(v >= 0);
            out_mesh->faceTexCoord.push_back(v);
            (*out_index_tuple)[i].textureIndex = v;
        } else {
            (*out_index_tuple)[i].textureIndex = 0;
        }
        //normal index (optional)
        if (out_mesh->hasNormal) {
            pos = pend + 1;
            if (*pos == '/') { ++pos; }
            v = static_cast<IndexType>(std::strtoll(pos, &pend, 0));
            if (v < 0) { v += max_normal_index; }
            GHULBUS_ASSERT(v >= 0);
            out_mesh->faceNormal.push_back(v);
            (*out_index_tuple)[i].normalIndex = v;
        } else {
            (*out_index_tuple)[i].normalIndex = 0;
        }
        //adjust position
        pos = pend + 1;
    }
    //adjust iterator
    if (pend != nullptr) { p += pend - &(*p); }
}

/** Retrieve a pointer to a FaceGroup from its name.
 * @param[in] name Group name.
 * @param[in,out] face_groups FaceGroups list; if no group of the given name exists, it will be created.
 * @param[in,out] group_names FaceGroupNames list; if no group of the given name exists, it will be created.
 */
inline ObjParser::FaceData*
    getFaceGroupByName(std::string const& name,
                       ObjParser::FaceGroups& face_groups,
                       ObjParser::FaceGroupNames& group_names)
{
    //get iterator to group name list
    auto it_name = std::find(group_names.begin(), group_names.end(), name);
    if (it_name == group_names.end()) {
        it_name = group_names.insert(it_name, name);
        face_groups.push_back(ObjParser::FaceData{});
    }
    GHULBUS_ASSERT((*it_name) == name);
    //get iterator to mesh data
    ObjParser::FaceGroups::iterator it_data = face_groups.begin();
    it_data += std::distance(group_names.begin(), it_name);
    return &(*it_data);
}

/** Process a group switch entry.
 * @param[in,out] p Iterator to beginning of line; On return, points to new location to proceed parsing from.
 * @param[in, out] face_groups Target list of FaceData.
 * @param[in, out] group_names Target GroupNameList.
 * @return A vector of pointers to all MeshData structs active in the current group.
 */
inline std::vector<ObjParser::FaceData*>
    groupSwitch(Buffer::const_iterator& p,
                ObjParser::FaceGroups& face_groups,
                ObjParser::FaceGroupNames& group_names)
{
    //skip 'g'
    ++p;
    std::vector<ObjParser::FaceData*> ret;
    while ((*p) != '\n') {
        //skip ' '
        ++p;
        //parse next group name
        Buffer::const_iterator group_name_start = p;
        while (((*p) != ' ') && ((*p) != '\n')) { 
            ++p; 
        }
        std::string const group_name(group_name_start, p);
        //add mesh data to return list
        ret.push_back(getFaceGroupByName(group_name, face_groups, group_names));
    }
    if (ret.empty()) {
        //default group
        std::string const group_name("default");
        ret.push_back(getFaceGroupByName(group_name, face_groups, group_names));
    }
    return ret;
}

/** Remove empty group entries.
 * @param[in,out] face_groups Group list; empty group entries will be removed.
 * @param[in,out] group_names Name list; names referring to empty groups will be removed.
 */
inline void removeEmptyGroups(ObjParser::FaceGroups& face_groups,
                              ObjParser::FaceGroupNames& group_names)
{
    ObjParser::FaceGroups::iterator it_face = face_groups.begin();
    ObjParser::FaceGroupNames::iterator it_name = group_names.begin();

    while (it_name != group_names.end()) {
        if (it_face->faceVertex.empty()) {
            it_face = face_groups.erase(it_face);
            it_name = group_names.erase(it_name);
        } else {
            ++it_face;
            ++it_name;
        }
    }
}

inline void addFlattenedFaces(std::vector<IndexTuple> const& index_tuples,
                              IndexTupleMap& index_tuple_map,
                              ObjParser::VertexData const& vertex_data,
                              ObjParser::VertexDataFlat& out_flat_vertex_data,
                              ObjParser::IndexDataFlat& out_flat_index_data)
{
    auto const it_end = index_tuples.end();
    for (auto it = index_tuples.begin(); it != it_end; ++it) {
        GHULBUS_ASSERT((it->vertexIndex > 0) && (it->vertexIndex <= vertex_data.vertex.size()));
        GHULBUS_ASSERT((vertex_data.normal.empty()) || (it->normalIndex <= vertex_data.normal.size()));
        GHULBUS_ASSERT(it->textureIndex <= vertex_data.texCoord.size());
        IndexTupleMap::iterator map_it = index_tuple_map.find(*it);
        if (map_it == index_tuple_map.end()) {
            /// index tuple does not exist; create a new vertex
            ObjParser::VertexEntryFlat v;
            size_t constexpr position = *ObjParser::VertexDataFlat::Format::getIndexForSemantics(VertexFormatBase::ComponentSemantics::Position);
            size_t constexpr normal = *ObjParser::VertexDataFlat::Format::getIndexForSemantics(VertexFormatBase::ComponentSemantics::Normal);
            size_t constexpr texture = *ObjParser::VertexDataFlat::Format::getIndexForSemantics(VertexFormatBase::ComponentSemantics::Texture);
            get<position>(v) = vertex_data.vertex[it->vertexIndex - 1];
            if (vertex_data.normal.empty()) {
                get<normal>(v) = GhulbusMath::Normal3f(1.0f, 0.0f, 0.0f);
            } else {
                get<normal>(v) = vertex_data.normal[it->normalIndex - 1];
            }
            if (vertex_data.texCoord.empty()) {
                get<texture>(v) = GhulbusMath::Vector2f(0.0f, 0.0f);
            } else {
                get<texture>(v) = vertex_data.texCoord[it->textureIndex - 1];
            }
            // add new vertex to flat data structures
            out_flat_vertex_data.getStorage().push_back(v);
            GHULBUS_ASSERT(out_flat_vertex_data.size() < std::numeric_limits<ObjParser::IndexType>::max());
            auto const index = static_cast<ObjParser::IndexType>(out_flat_vertex_data.size()) - 1;
            index_tuple_map[(*it)] = index;
            out_flat_index_data.push_back(index);
        } else {
            /// index tuple does exist; reuse existing flat vertex
            out_flat_index_data.push_back(map_it->second);
        }
    }
    if (index_tuples.size() == 4) {
        ///Quad mesh; Add second triangle
        out_flat_index_data.push_back(out_flat_index_data[out_flat_index_data.size() - 4]);
        out_flat_index_data.push_back(out_flat_index_data[out_flat_index_data.size() - 3]);
    }
}

/**
 * @param[in] data Buffer holding a complete OBJ file.
 * @param[out] out_mesh Processed mesh data.
 */
void parseData(Buffer const& data,
               ObjParser::VertexData& out_vertex_data,
               ObjParser::FaceGroups& out_face_groups,
               ObjParser::FaceGroupNames& out_group_names,
               ObjParser::VertexDataFlat& out_flat_vertex_data,
               ObjParser::IndexDataFlat& out_flat_index_data)
{
    Buffer::const_iterator const eof = data.end();
    Buffer::const_iterator position = data.begin();
    std::vector<ObjParser::FaceData*> face_target_groups;
    IndexTupleMap index_tuple_map;
    std::vector<IndexTuple> tmp_tuple;
    int line_index = 1;
    while (position != eof) {
        switch (*position) {
        case '#': /* comment; skip line */  break;
        case '\0': /* end of string */  break;
        case 'g':
            face_target_groups = groupSwitch(position, out_face_groups, out_group_names);
            if (face_target_groups.size() != 1) {
                GHULBUS_THROW(Exceptions::NotImplemented(), "Multiple target groups not supported.");
            }
            break;
        case 'v':
            parseVertex(position, &out_vertex_data);
            break;
        case 'f':
            if (face_target_groups.empty()) {
                face_target_groups.push_back(getFaceGroupByName("default", out_face_groups, out_group_names));
            }
            if (face_target_groups.front()->verticesPerFace <= 0) {
                scanFaceLayout(position, eof, face_target_groups.front());
                tmp_tuple.resize(face_target_groups.front()->verticesPerFace, IndexTuple { 0, 0, 0 });
            }
            parseFace(position, face_target_groups.front(), &tmp_tuple, out_vertex_data);
            GHULBUS_ASSERT( ((*position) == '\n') ||
                            ((*position) == '\r') ||
                            ((*skipWhitespace(position, eof)) == '\n') );
            addFlattenedFaces( tmp_tuple, index_tuple_map, out_vertex_data,
                               out_flat_vertex_data, out_flat_index_data );
            break;
        default:
            GHULBUS_LOG(Warning, "Unknown label: \'" << (*position) << "\' at line " << line_index);
            break;
        }
        //advance to next line
        position = skipWhitespace(getEndOfLine(position, eof), eof, &line_index);
    }
    //remove empty groups
    removeEmptyGroups(out_face_groups, out_group_names);
    GHULBUS_LOG(Debug, "OBJ Loader processed total of " << line_index << " lines.");
}
}
}
