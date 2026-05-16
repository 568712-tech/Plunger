#pragma once

#include "renderer/Mesh.h"
#include "scene/GltfModel.h"

#include <filesystem>

namespace plunger {

class ModelLoader {
public:
    static Mesh loadObj(const std::filesystem::path& path);
    static GltfModel loadGltf(const std::filesystem::path& path);
};

} // namespace plunger