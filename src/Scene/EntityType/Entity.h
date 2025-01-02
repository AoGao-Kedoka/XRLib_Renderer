#pragma once

#include "Utils/Transform.h"

namespace XRLib {
class Entity {
   public:
    Entity(Transform transform, const std::string& name) : transform{transform}, name{name} {}
    Entity(Transform transform) : transform{transform}, name{"DefaultEntity}"} {}
    Entity(std::string name) : transform{}, name{name} {}
    Entity() : transform{}, name{"DefaultEntity"} {}
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

    Transform& GetLocalTransform() { return transform; }
    const Transform GetGlobalTransform() {
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

    template <typename T>
    static constexpr void AddEntity(std::unique_ptr<T>& entity, Entity* parent,
                                    std::vector<T*>* renderReferenceVec = nullptr) {
        static_assert(std::is_base_of<Entity, T>::value, "T must be a child of Entity");

        if (renderReferenceVec) {
            renderReferenceVec->push_back(entity.get());
        }

        entity->SetParent(parent);
        parent->GetChilds().push_back(std::move(entity));
    }

    template <typename T>
    static constexpr void AddEntity(std::unique_ptr<T>& entity, std::vector<std::unique_ptr<Entity>>& hiearchy,
                                    std::vector<T*>* renderReferenceVec = nullptr) {
        static_assert(std::is_base_of<Entity, T>::value, "T must be a child of Entity");

        if (renderReferenceVec) {
            renderReferenceVec->push_back(entity.get());
        }
        entity->SetParent(nullptr);
        hiearchy.push_back(std::move(entity));
    }

    template <typename T>
    T* ToType(Entity* entity) {
        static_assert(std::is_base_of<Entity, T>::value, "T must be a child of Entity");
        return dynamic_cast<T*>(entity);
    }

   protected:
    std::string name;
    std::vector<std::unique_ptr<Entity>> childs;
    Entity* parent{nullptr};
    Transform transform;
    std::vector<TAG> tags;
};

}    // namespace XRLib