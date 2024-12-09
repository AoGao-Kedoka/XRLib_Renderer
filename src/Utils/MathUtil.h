#pragma once

#include <pch.h>

namespace XRLib {
class MathUtil {
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

    static glm::mat4 XrPoseToMatrix(const XrPosef& pose) {
        const glm::mat4 translation =
            glm::translate(glm::mat4(1.0f), glm::vec3(pose.position.x, pose.position.y, pose.position.z));

        const glm::mat4 rotation =
            glm::toMat4(glm::quat(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z));

        return translation * rotation;
    }

    static glm::mat4 XrCreateProjectionMatrix(XrFovf fov, float nearClip, float farClip) {
        const float l = glm::tan(fov.angleLeft);
        const float r = glm::tan(fov.angleRight);
        const float d = glm::tan(fov.angleDown);
        const float u = glm::tan(fov.angleUp);

        const float w = r - l;
        const float h = d - u;

        glm::mat4 projectionMatrix;
        projectionMatrix[0] = {2.0f / w, 0.0f, 0.0f, 0.0f};
        projectionMatrix[1] = {0.0f, 2.0f / h, 0.0f, 0.0f};
        projectionMatrix[2] = {(r + l) / w, (u + d) / h, -(farClip + nearClip) / (farClip - nearClip), -1.0f};
        projectionMatrix[3] = {0.0f, 0.0f, -(farClip * (nearClip + nearClip)) / (farClip - nearClip), 0.0f};
        return projectionMatrix;
    }

    static glm::mat4 GetTransformationMatrix(glm::vec3 translation, glm::vec3 rotation, float rotationRadians,
                                             glm::vec3 scale) {
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, translation);
        if (rotationRadians != 0)
            trans = glm::rotate(trans, glm::radians(rotationRadians), rotation);
        trans = glm::scale(trans, scale);
        return trans;
    }
};
}    // namespace XRLib
