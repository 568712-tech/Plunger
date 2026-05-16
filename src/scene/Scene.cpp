#include "scene/Scene.h"

#include <algorithm>

namespace plunger {

EntityId Scene::createEntity()
{
    const EntityId entityId = m_nextEntityId++;
    m_entities.push_back({entityId, 0});
    return entityId;
}

void Scene::destroyEntity(EntityId entity)
{
    m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(), [entity](const Entity& current) {
        return current.id == entity;
    }), m_entities.end());

    if (auto transform = m_transforms.find(entity); transform != m_transforms.end()) {
        if (transform->second.parent != InvalidEntity) {
            if (Transform* parent = getTransform(transform->second.parent); parent != nullptr) {
                parent->children.erase(
                    std::remove(parent->children.begin(), parent->children.end(), entity),
                    parent->children.end());
            }
        }
        m_transforms.erase(transform);
    }

    m_meshRenderers.erase(entity);
}

Transform& Scene::addTransform(EntityId entity)
{
    Transform& transform = m_transforms[entity];
    transform.dirty = true;

    for (Entity& current : m_entities) {
        if (current.id == entity) {
            current.mask |= static_cast<std::uint32_t>(ComponentMask::Transform);
            break;
        }
    }

    return transform;
}

MeshRenderer& Scene::addMeshRenderer(EntityId entity, const Mesh* mesh, const Texture* texture)
{
    MeshRenderer& meshRenderer = m_meshRenderers[entity];
    meshRenderer.mesh = mesh;
    meshRenderer.texture = texture;

    for (Entity& current : m_entities) {
        if (current.id == entity) {
            current.mask |= static_cast<std::uint32_t>(ComponentMask::MeshRenderer);
            break;
        }
    }

    return meshRenderer;
}

Part& Scene::addPart(EntityId entity, const Part& part)
{
    Part& storedPart = m_parts[entity];
    storedPart = part;

    for (Entity& current : m_entities) {
        if (current.id == entity) {
            current.mask |= static_cast<std::uint32_t>(ComponentMask::Part);
            break;
        }
    }

    return storedPart;
}

Transform* Scene::getTransform(EntityId entity)
{
    if (auto iter = m_transforms.find(entity); iter != m_transforms.end()) {
        return &iter->second;
    }

    return nullptr;
}

const Transform* Scene::getTransform(EntityId entity) const
{
    if (auto iter = m_transforms.find(entity); iter != m_transforms.end()) {
        return &iter->second;
    }

    return nullptr;
}

MeshRenderer* Scene::getMeshRenderer(EntityId entity)
{
    if (auto iter = m_meshRenderers.find(entity); iter != m_meshRenderers.end()) {
        return &iter->second;
    }

    return nullptr;
}

const MeshRenderer* Scene::getMeshRenderer(EntityId entity) const
{
    if (auto iter = m_meshRenderers.find(entity); iter != m_meshRenderers.end()) {
        return &iter->second;
    }

    return nullptr;
}

Part* Scene::getPart(EntityId entity)
{
    if (auto iter = m_parts.find(entity); iter != m_parts.end()) {
        return &iter->second;
    }

    return nullptr;
}

const Part* Scene::getPart(EntityId entity) const
{
    if (auto iter = m_parts.find(entity); iter != m_parts.end()) {
        return &iter->second;
    }

    return nullptr;
}

void Scene::setParent(EntityId child, EntityId parent)
{
    Transform* childTransform = getTransform(child);
    if (childTransform == nullptr) {
        return;
    }

    if (childTransform->parent != InvalidEntity) {
        if (Transform* previousParent = getTransform(childTransform->parent); previousParent != nullptr) {
            previousParent->children.erase(
                std::remove(previousParent->children.begin(), previousParent->children.end(), child),
                previousParent->children.end());
        }
    }

    childTransform->parent = parent;
    if (parent != InvalidEntity) {
        if (Transform* parentTransform = getTransform(parent); parentTransform != nullptr) {
            parentTransform->children.push_back(child);
        }
    }

    markDirty(child);
}

void Scene::updateTransforms()
{
    for (const Entity& entity : m_entities) {
        if (auto transform = m_transforms.find(entity.id); transform != m_transforms.end() && transform->second.parent == InvalidEntity) {
            updateWorldRecursive(entity.id, Mat4::identity());
        }
    }
}

void Scene::markDirty(EntityId entity)
{
    if (Transform* transform = getTransform(entity); transform != nullptr) {
        transform->dirty = true;
        for (EntityId child : transform->children) {
            markDirty(child);
        }
    }
}

void Scene::updateWorldRecursive(EntityId entity, const Mat4& parentWorld)
{
    Transform* transform = getTransform(entity);
    if (transform == nullptr) {
        return;
    }

    if (transform->dirty) {
        transform->rebuildLocalMatrix();
    }

    transform->worldMatrix = multiply(parentWorld, transform->localMatrix);
    transform->dirty = false;

    for (EntityId child : transform->children) {
        updateWorldRecursive(child, transform->worldMatrix);
    }
}

} // namespace plunger