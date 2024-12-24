#pragma once

#include "Utils/Transform.h"

namespace XRLib {
class Entity {
   public:
    Entity(Transform transform, std::string name) : transform{transform}, name{name} {}
    Entity(Transform transform) : transform{transform}, name{"DefaultEntity}"} {}
    Entity(std::string name) : transform{{}}, name{name} {}
    Entity() : transform{{}}, name{"DefaultEntity"} {}
    ~Entity() = default;

    // std::vector<Entity*>& GetChilds() { return childs; }
    Entity* GetParent() { return parent; }
    bool IsRoot() { return parent == nullptr; }

    Transform GetRelativeTransform() { return transform; }
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

   protected:
    std::string name;
    std::vector<std::unique_ptr<Entity>> childs;
    Entity* parent{nullptr};
    Transform transform;
};

}    // namespace XRLib