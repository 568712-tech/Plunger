#pragma once

#include "scene/Components.h"

#include <filesystem>
#include <vector>

namespace plunger {

class PartMapLoader {
public:
    static std::vector<Part> loadJson(const std::filesystem::path& path);
};

} // namespace plunger