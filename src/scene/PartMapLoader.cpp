#include "scene/PartMapLoader.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace plunger {
namespace {

constexpr float pi = 3.14159265f;

std::string readTextFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open part map: " + path.string());
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

std::vector<float> parseFloatArray(const std::string& objectText, const std::string& key)
{
    const std::regex pattern("\"" + key + "\"\\s*:\\s*\\[([^\\]]*)\\]");
    std::smatch match;
    if (!std::regex_search(objectText, match, pattern)) {
        throw std::runtime_error("Part map missing array field: " + key);
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

float parseOptionalFloat(const std::string& objectText, const std::string& key, float fallback)
{
    const std::regex pattern("\"" + key + "\"\\s*:\\s*([-+]?\\d*\\.?\\d+(?:[eE][-+]?\\d+)?)");
    std::smatch match;
    if (!std::regex_search(objectText, match, pattern)) {
        return fallback;
    }

    return std::stof(match[1].str());
}

std::vector<float> parseOptionalFloatArray(const std::string& objectText, const std::string& key)
{
    const std::regex pattern("\"" + key + "\"\\s*:\\s*\\[([^\\]]*)\\]");
    std::smatch match;
    if (!std::regex_search(objectText, match, pattern)) {
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

std::string parseOptionalString(const std::string& objectText, const std::string& key)
{
    const std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;
    if (!std::regex_search(objectText, match, pattern)) {
        return {};
    }

    return match[1].str();
}

PartShape parsePartShape(const std::string& shapeName)
{
    if (shapeName.empty() || shapeName == "box") {
        return PartShape::Box;
    }
    if (shapeName == "cylinder") {
        return PartShape::Cylinder;
    }
    if (shapeName == "pyramid") {
        return PartShape::Pyramid;
    }
    if (shapeName == "wedge") {
        return PartShape::Wedge;
    }
    if (shapeName == "cone") {
        return PartShape::Cone;
    }
    if (shapeName == "sphere") {
        return PartShape::Sphere;
    }
    if (shapeName == "hemisphere" || shapeName == "half_sphere" || shapeName == "dome") {
        return PartShape::Hemisphere;
    }
    if (shapeName == "torus" || shapeName == "donut") {
        return PartShape::Torus;
    }
    if (shapeName == "tri_prism" || shapeName == "triangular_prism" || shapeName == "prism") {
        return PartShape::TriPrism;
    }
    if (shapeName == "hex_prism" || shapeName == "hexagonal_prism") {
        return PartShape::HexPrism;
    }
    if (shapeName == "octahedron" || shapeName == "diamond") {
        return PartShape::Octahedron;
    }
    if (shapeName == "capsule" || shapeName == "pill") {
        return PartShape::Capsule;
    }

    throw std::runtime_error("Unsupported part shape: " + shapeName);
}

float degreesToRadians(float degrees)
{
    return degrees * pi / 180.f;
}

std::string extractArrayBody(const std::string& text, const std::string& key)
{
    const std::string keyToken = '"' + key + '"';
    const std::size_t keyPosition = text.find(keyToken);
    if (keyPosition == std::string::npos) {
        throw std::runtime_error("Part map missing parts array");
    }

    const std::size_t openBracket = text.find('[', keyPosition + keyToken.size());
    if (openBracket == std::string::npos) {
        throw std::runtime_error("Part map missing parts array");
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

    throw std::runtime_error("Part map missing parts array");
}

std::vector<std::string> extractObjects(const std::string& text)
{
    const std::string partsText = extractArrayBody(text, "parts");
    std::vector<std::string> objects;
    std::size_t depth = 0;
    std::size_t objectStart = std::string::npos;

    for (std::size_t index = 0; index < partsText.size(); ++index) {
        if (partsText[index] == '{') {
            if (depth == 0) {
                objectStart = index;
            }
            ++depth;
        } else if (partsText[index] == '}') {
            --depth;
            if (depth == 0 && objectStart != std::string::npos) {
                objects.push_back(partsText.substr(objectStart, index - objectStart + 1));
                objectStart = std::string::npos;
            }
        }
    }

    return objects;
}

} // namespace

std::vector<Part> PartMapLoader::loadJson(const std::filesystem::path& path)
{
    const std::string text = readTextFile(path);
    const std::vector<std::string> objects = extractObjects(text);
    std::vector<Part> parts;
    parts.reserve(objects.size());

    for (const std::string& objectText : objects) {
        Part part;
        const std::vector<float> position = parseFloatArray(objectText, "position");
        const std::vector<float> size = parseFloatArray(objectText, "size");
        const std::vector<float> color = parseFloatArray(objectText, "color");

        if (position.size() != 3u || size.size() != 3u || color.size() != 3u) {
            throw std::runtime_error("Each part must define position, size, and color arrays of length 3: " + path.string());
        }

        part.position = {position[0], position[1], position[2]};
        part.size = {size[0], size[1], size[2]};
        const std::vector<float> rotation = parseOptionalFloatArray(objectText, "rotation");
        if (!rotation.empty() && rotation.size() != 3u) {
            throw std::runtime_error("Part rotation must be an array of length 3: " + path.string());
        }

        if (!rotation.empty()) {
            part.rotation = {
                degreesToRadians(rotation[0]),
                degreesToRadians(rotation[1]),
                degreesToRadians(rotation[2]),
            };
        }

        part.bobAmplitude = parseOptionalFloat(objectText, "bobAmplitude", 0.f);
        part.circleRadius = parseOptionalFloat(objectText, "circleRadius", 0.f);
        part.circleSpeed = parseOptionalFloat(objectText, "circleSpeed", 1.0f);
        part.circleAxisY = parseOptionalFloat(objectText, "circleAxisY", 1.0f) > 0.5f;
        part.swayAmplitude = parseOptionalFloat(objectText, "swayAmplitude", 0.f);
        part.swaySpeed = parseOptionalFloat(objectText, "swaySpeed", 1.0f);
        part.spinSpeed = parseOptionalFloat(objectText, "spinSpeed", 0.f);
        part.shape = parsePartShape(parseOptionalString(objectText, "shape"));
        part.material.baseColor = {color[0], color[1], color[2]};
        part.material.roughness = parseOptionalFloat(objectText, "roughness", 1.f);
        part.material.metallic = parseOptionalFloat(objectText, "metallic", 0.f);
        part.material.texturePath = parseOptionalString(objectText, "texture");
        parts.push_back(part);
    }

    return parts;
}

} // namespace plunger
