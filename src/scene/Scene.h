#pragma once

#include "scene/Components.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace plunger {

class Scene {
public:
    EntityId createEntity();
    void destroyEntity(EntityId entity);

    Transform& addTransform(EntityId entity);
    MeshRenderer& addMeshRenderer(EntityId entity, const Mesh* mesh, const Texture* texture);
    Part& addPart(EntityId entity, const Part& part);

    Transform* getTransform(EntityId entity);
    const Transform* getTransform(EntityId entity) const;
    MeshRenderer* getMeshRenderer(EntityId entity);
    const MeshRenderer* getMeshRenderer(EntityId entity) const;
    Part* getPart(EntityId entity);
    const Part* getPart(EntityId entity) const;

    void setParent(EntityId child, EntityId parent);
    void updateTransforms();

    template <typename Callback>
    void forEachRenderable(Callback&& callback) const
    {
        for (const Entity& entity : m_entities) {
            if (!hasMask(entity.mask, ComponentMask::Transform | ComponentMask::MeshRenderer)) {
                continue;
            }

            const Transform* transform = getTransform(entity.id);
            const MeshRenderer* meshRenderer = getMeshRenderer(entity.id);
            if (transform != nullptr && meshRenderer != nullptr && meshRenderer->mesh != nullptr) {
                callback(entity, *transform, *meshRenderer);
            }
        }
    }

    template <typename Callback>
    void forEachPart(Callback&& callback) const
    {
        for (const Entity& entity : m_entities) {
            if (!hasMask(entity.mask, ComponentMask::Part)) {
                continue;
            }

            const Part* part = getPart(entity.id);
            if (part != nullptr) {
                callback(entity, *part);
            }
        }
    }

    bool empty() const
    {
        return m_entities.empty();
    }

private:
    void markDirty(EntityId entity);
    void updateWorldRecursive(EntityId entity, const Mat4& parentWorld);

    EntityId m_nextEntityId = 1;
    std::vector<Entity> m_entities;
    std::unordered_map<EntityId, Transform> m_transforms;
    std::unordered_map<EntityId, MeshRenderer> m_meshRenderers;
    std::unordered_map<EntityId, Part> m_parts;
};

} // namespace plunger