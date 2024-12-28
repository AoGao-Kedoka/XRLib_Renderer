#pragma once

#include "Entity.h"

namespace XRLib {
class PointLight : public Entity {
   public:
    PointLight(Transform transform, glm::vec4 color, float intensity, const std::string& name = "DefaultLight")
        : color{color}, intensity{intensity}, Entity{transform, name} {}

    glm::vec4& GetColor() { return color; }
    float& GetIntensity() { return intensity; }

   private:
    float intensity;
    glm::vec4 color;
};
}    // namespace XRLib