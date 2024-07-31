#pragma once

#include <math.h>
#include <openxr/openxr.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

class LAMath {
   public:
    static XrPosef Identity() {
        XrPosef pose{};
        pose.orientation.w = 1;
        return pose;
    }

    static XrPosef Translation(const XrVector3f& translation) {
        XrPosef t = Identity();
        t.position = translation;
        return t;
    }

    static XrPosef RotateCCWAboutYAxis(float radians, XrVector3f translation) {
        XrPosef t = Identity();
        t.orientation.x = 0.f;
        t.orientation.y = std::sin(radians * 0.5f);
        t.orientation.z = 0.f;
        t.orientation.w = std::cos(radians * 0.5f);
        t.position = translation;
        return t;
    }

    static glm::mat4 GetTransformationMatrix(glm::vec3 translation,
        glm::vec3 rotation,
        float rotationRadians,
        glm::vec3 scale) {
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, translation);
        trans = glm::rotate(trans, glm::radians(rotationRadians), rotation);
        trans = glm::scale(trans, scale);
        return trans;
    }
};
