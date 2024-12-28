#pragma once

#include "Scene.h"

namespace XRLib {
template <typename T>
void Scene::AddEntityInternal(std::unique_ptr<T>& entity, Entity* parent, std::vector<T*>* renderReferenceVec) {
    static_assert(std::is_base_of<Entity, T>::value, "T must be a child of Entity");

    if (renderReferenceVec) {
        renderReferenceVec->push_back(entity.get());
    }

    entity->SetParent(parent);

    if (parent == nullptr) {
        sceneHierarchy.push_back(std::move(entity));
    } else {
        parent->GetChilds().push_back(std::move(entity));
    }
}
}
