#include "scene/ModelLoader.h"
#include "scene/GltfModel.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace plunger {
namespace {

struct VertexKey {
    int positionIndex = 0;
    int texCoordIndex = 0;
    int normalIndex = 0;

    bool operator==(const VertexKey& other) const
    {
        return positionIndex == other.positionIndex && texCoordIndex == other.texCoordIndex && normalIndex == other.normalIndex;
    }
};

struct VertexKeyHasher {
    std::size_t operator()(const VertexKey& key) const noexcept
    {
        std::size_t seed = static_cast<std::size_t>(key.positionIndex);
        seed = seed * 1315423911u + static_cast<std::size_t>(key.texCoordIndex);
        seed = seed * 1315423911u + static_cast<std::size_t>(key.normalIndex);
        return seed;
    }
};

int parseIndex(const std::string& token, std::size_t& cursor)
{
    const std::size_t start = cursor;
    while (cursor < token.size() && token[cursor] != '/') {
        ++cursor;
    }

    const std::string value = token.substr(start, cursor - start);
    if (value.empty()) {
        return 0;
    }

    return std::stoi(value);
}

VertexKey parseFaceVertex(const std::string& token)
{
    std::size_t cursor = 0;
    VertexKey key;
    key.positionIndex = parseIndex(token, cursor);
    if (cursor < token.size() && token[cursor] == '/') {
        ++cursor;
        key.texCoordIndex = parseIndex(token, cursor);
        if (cursor < token.size() && token[cursor] == '/') {
            ++cursor;
            key.normalIndex = parseIndex(token, cursor);
        }
    }

    return key;
}

template <typename T>
T resolveIndex(const std::vector<T>& values, int index)
{
    if (index > 0) {
        return values[static_cast<std::size_t>(index - 1)];
    }

    if (index < 0) {
        return values[static_cast<std::size_t>(values.size() + index)];
    }

    return T{};
}

} // namespace

Mesh ModelLoader::loadObj(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open OBJ file: " + path.string());
    }

    std::vector<Vec3> positions;
    std::vector<Vec2> texCoords;
    std::vector<Vec3> normals;
    std::vector<Mesh::Vertex> vertices;
    std::vector<unsigned int> indices;
    std::unordered_map<VertexKey, unsigned int, VertexKeyHasher> vertexLookup;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream stream(line);
        std::string prefix;
        stream >> prefix;

        if (prefix == "v") {
            Vec3 position;
            stream >> position.x >> position.y >> position.z;
            positions.push_back(position);
        } else if (prefix == "vt") {
            Vec2 texCoord;
            stream >> texCoord.x >> texCoord.y;
            texCoords.push_back(texCoord);
        } else if (prefix == "vn") {
            Vec3 normal;
            stream >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } else if (prefix == "f") {
            std::vector<VertexKey> faceVertices;
            std::string token;
            while (stream >> token) {
                faceVertices.push_back(parseFaceVertex(token));
            }

            if (faceVertices.size() < 3) {
                continue;
            }

            for (std::size_t triangleIndex = 1; triangleIndex + 1 < faceVertices.size(); ++triangleIndex) {
                const VertexKey triangleKeys[3] = {faceVertices[0], faceVertices[triangleIndex], faceVertices[triangleIndex + 1]};
                for (const VertexKey& key : triangleKeys) {
                    auto found = vertexLookup.find(key);
                    if (found == vertexLookup.end()) {
                        Mesh::Vertex vertex;
                        vertex.position = resolveIndex(positions, key.positionIndex);
                        vertex.texCoord = resolveIndex(texCoords, key.texCoordIndex);
                        vertex.normal = resolveIndex(normals, key.normalIndex);

                        const unsigned int newIndex = static_cast<unsigned int>(vertices.size());
                        vertices.push_back(vertex);
                        vertexLookup.emplace(key, newIndex);
                        indices.push_back(newIndex);
                    } else {
                        indices.push_back(found->second);
                    }
                }
            }
        }
    }

    if (vertices.empty() || indices.empty()) {
        throw std::runtime_error("OBJ file contained no renderable geometry: " + path.string());
    }

    return Mesh::fromData(std::move(vertices), std::move(indices));
}

GltfModel ModelLoader::loadGltf(const std::filesystem::path& path)
{
    return GltfModelLoader::load(path);
}

} // namespace plunger