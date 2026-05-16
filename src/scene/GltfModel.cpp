#include "scene/GltfModel.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace plunger {
namespace {

std::string readTextFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open glTF file: " + path.string());
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

std::vector<float> parseFloatArray(const std::string& text, const std::string& key)
{
    const std::regex pattern("\"" + key + "\"\\s*:\\s*\\[([^\\]]*)\\]");
    std::smatch match;
    if (!std::regex_search(text, match, pattern)) {
        return {};
    }

    std::vector<float> values;
    std::istringstream stream(match[1].str());
    std::string token;
    while (std::getline(stream, token, ',')) {
        const std::size_t begin = token.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos) {
            continue;
        }

        const std::size_t end = token.find_last_not_of(" \t\r\n");
        values.push_back(std::stof(token.substr(begin, end - begin + 1)));
    }

    return values;
}

std::string extractArrayBody(const std::string& text, const std::string& key)
{
    const std::string keyToken = '"' + key + '"';
    const std::size_t keyPosition = text.find(keyToken);
    if (keyPosition == std::string::npos) {
        return {};
    }

    const std::size_t openBracket = text.find('[', keyPosition + keyToken.size());
    if (openBracket == std::string::npos) {
        return {};
    }

    std::size_t depth = 0;
    for (std::size_t index = openBracket; index < text.size(); ++index) {
        if (text[index] == '[') {
            ++depth;
        } else if (text[index] == ']') {
            --depth;
            if (depth == 0) {
                return text.substr(openBracket + 1, index - openBracket - 1);
            }
        }
    }

    return {};
}

std::vector<GltfMaterial> parseMaterials(const std::string& text)
{
    std::vector<GltfMaterial> materials;
    const std::string materialsText = extractArrayBody(text, "materials");
    if (materialsText.empty()) {
        return materials;
    }

    std::size_t depth = 0;
    std::size_t objectStart = std::string::npos;

    for (std::size_t index = 0; index < materialsText.size(); ++index) {
        if (materialsText[index] == '{') {
            if (depth == 0) {
                objectStart = index;
            }
            ++depth;
        } else if (materialsText[index] == '}') {
            --depth;
            if (depth == 0 && objectStart != std::string::npos) {
                const std::string materialText = materialsText.substr(objectStart, index - objectStart + 1);
                GltfMaterial material;

                const std::vector<float> baseColorFactor = parseFloatArray(materialText, "baseColorFactor");
                if (baseColorFactor.size() == 4u) {
                    material.material.baseColor = {baseColorFactor[0], baseColorFactor[1], baseColorFactor[2]};
                }

                const std::vector<float> pbrValues = parseFloatArray(materialText, "pbrMetallicRoughness");
                (void)pbrValues;

                const std::regex roughnessPattern("\"roughnessFactor\"\\s*:\\s*([-+]?\\d*\\.?\\d+(?:[eE][-+]?\\d+)?)");
                const std::regex metallicPattern("\"metallicFactor\"\\s*:\\s*([-+]?\\d*\\.?\\d+(?:[eE][-+]?\\d+)?)");
                std::smatch match;
                if (std::regex_search(materialText, match, roughnessPattern)) {
                    material.material.roughness = std::stof(match[1].str());
                }
                if (std::regex_search(materialText, match, metallicPattern)) {
                    material.material.metallic = std::stof(match[1].str());
                }

                materials.push_back(material);
                objectStart = std::string::npos;
            }
        }
    }

    return materials;
}

} // namespace

GltfModel GltfModelLoader::load(const std::filesystem::path& path)
{
    const std::string text = readTextFile(path);
    if (text.find("\"asset\"") == std::string::npos) {
        throw std::runtime_error("Unsupported glTF file: " + path.string());
    }

    GltfModel model;
    model.materials = parseMaterials(text);
    model.mesh = Mesh::createCube();
    return model;
}

} // namespace plunger