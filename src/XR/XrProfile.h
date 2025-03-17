#pragma once

#include "pch.h"

namespace XRLib {
namespace XR {
struct XrProfile {
    enum InteractionProfile {
        KHR_SIMPLE_CONTROLLER,
        VALVE_INDEX_CONTROLLER,
        OCULUS_TOUCH_CONTROLLER,
    };

    // https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#semantic-path-interaction-profiles
    struct InteractionProfileInfo {
        std::string path;

        struct UserPath {
            std::string left;
            std::string right;
        } userPath;

        std::vector<std::string> commonSupportedComponentPaths;
        std::vector<std::string> leftHandComponentPaths = {};
        std::vector<std::string> rightHandcomponentPath = {};
    };

    std::unordered_map<InteractionProfile, InteractionProfileInfo> interactionProfileMap{

        {KHR_SIMPLE_CONTROLLER,
         {"/interaction_profiles/khr/simple_controller",
          {"/user/hand/left", "/user/hand/right"},
          {

              "input/select/click", "input/menu/click", "input/grip/pose", "input/aim/pose", "output/haptic"

          }}},

        {VALVE_INDEX_CONTROLLER,
         {"/interaction_profiles/valve/index_controller",
          {"/user/hand/left", "user/hand/right"},
          {

              "/input/system/click", "/input/squeeze/click", "/input/menu/click", "/input/trigger/click",
              "/input/trigger/value", "/input/trackpad/x", "/input/trackpad/y", "input/trackpad/click",
              "/input/trackpad/touch", "/input/grip/pose", "/input/aim/pose", "/output/haptic"

          }}},

        {OCULUS_TOUCH_CONTROLLER,
         {"/interaction_profiles/oculus/touch_controller",
          {"/user/hand/left", "/user/hand/right"},
          {

              "/input/squeeze/value", "/input/trigger/value", "/input/trigger/touch", "/input/thumbstick/x",
              "/input/thumbstick/y", "/input/thumbstick/click", "/input/thumbstick/touch", "/input/thumbrest/touch",
              "/input/grip/pose", "/input/aim/pose", "/output/haptic"

          },
          {

              "/input/x/click", "/input/x/touch", "/input/y/click", "/input/y/touch", "/input/menu/click"

          },
          {

              "/input/a/click", "/input/a/touch", "/input/b/click", "/input/b/touch", "/input/system/click"

          }}},
    };
};
}    // namespace XR
}    // namespace XRLib
