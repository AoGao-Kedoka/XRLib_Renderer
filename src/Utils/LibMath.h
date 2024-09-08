#pragma once

#include <pch.h>

class LibMath {
   public:
    static XrPosef XrIdentity() {
        XrPosef pose{};
        pose.orientation.w = 1;
        return pose;
    }

    static XrPosef XrTranslation(const XrVector3f& translation) {
        XrPosef t = XrIdentity();
        t.position = translation;
        return t;
    }

    static XrPosef XrRotateCCWAboutYAxis(float radians, XrVector3f translation) {
        XrPosef t = XrIdentity();
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
        if (rotationRadians != 0)
            trans = glm::rotate(trans, glm::radians(rotationRadians), rotation);
        trans = glm::scale(trans, scale);
        return trans;
    }
};
