#pragma once

#include "Utils/Transform.h"

namespace XRLib {
class Entity {
   public:
    Entity(Transform transform, const std::string& name) : transform{transform}, name{name} {}
    Entity(Transform transform) : transform{transform}, name{"DefaultEntity}"} {}
    Entity(std::string name) : transform{{}}, name{name} {}
    Entity() : transform{{}}, name{"DefaultEntity"} {}
    ~Entity() = default;

    enum TAG {
        MAIN_CAMERA,
        MESH_LEFT_CONTROLLER,
        MESH_RIGHT_CONTROLLER,
    };

    std::vector<std::unique_ptr<Entity>>& GetChilds() { return childs; }
    Entity* GetParent() { return parent; }
    void SetParent(Entity* parent) { this->parent = parent; }
    bool IsRoot() { return parent == nullptr; }

    Transform& GetRelativeTransform() { return transform; }
    Transform GetGlobalTransform() {
        Transform globalTransform = transform;
        Entity* current = this;

        while (current->parent != nullptr) {
            globalTransform = current->parent->transform * globalTransform;
            current = current->parent;
        }

        return globalTransform;
    }

    const std::string& GetName() { return name; }
    void Rename(const std::string& n) { name = n; }
    std::vector<TAG>& Tags() { return tags; }

   protected:
    std::string name;
    std::vector<std::unique_ptr<Entity>> childs;
    Entity* parent{nullptr};
    Transform transform;
    std::vector<TAG> tags;
};

}    // namespace XRLib