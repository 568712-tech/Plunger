#pragma once

#include "renderer/Mesh.h"
#include "scene/Components.h"

#include <filesystem>
#include <vector>

namespace plunger {

struct GltfMaterial {
    Material material;
};

struct GltfModel {
    Mesh mesh;
    std::vector<GltfMaterial> materials;
};

class GltfModelLoader {
public:
    static GltfModel load(const std::filesystem::path& path);
};

} // namespace plunger