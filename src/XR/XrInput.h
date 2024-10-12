#pragma once

#include "Event/EventSystem.h"
#include "Event/Events.h"
#include "Utils/Transform.h"
#include "XrCore.h"

namespace XRLib {
namespace XR {
class XrInput {
   public:
    XrInput(std::shared_ptr<XrCore> core);
    XrInput() = default;
    ~XrInput() = default;
    void UpdateInput();
   private:
    std::shared_ptr<XrCore> core;
    std::string suggestedInteractionProfile = "/interaction_profiles/khr/simple_controller";

    XrActionSet actionSet{XR_NULL_HANDLE};
    XrAction controllerPoseAction{XR_NULL_HANDLE};
    XrSpace leftHandSpace{XR_NULL_HANDLE}, rightHandSpace{XR_NULL_HANDLE};
};
}    // namespace XR
}    // namespace XRLib
