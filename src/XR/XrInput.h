#pragma once

#include "Event/EventSystem.h"
#include "Event/Events.h"
#include "Utils/Transform.h"
#include "XrCore.h"
#include "XrProfile.h"

namespace XRLib {
namespace XR {
class XrInput {
   public:
    XrInput(XrCore& core, XrProfile::InteractionProfile profile = XrProfile::KHR_SIMPLE_CONTROLLER);
    XrInput() = default;
    ~XrInput() = default;
    void UpdateInput();

   private:
    void CreateDefaultInteractionActionBindings();
    void UpdatePosePosition();
    void UpdateTriggerValue();
    void UpdateGripValue();

   private:
    XrCore* core{nullptr};
    XrProfile profile;
    XrProfile::InteractionProfile selectedProfile;

    XrActionSet actionSet{XR_NULL_HANDLE};

    XrAction controllerPoseAction{XR_NULL_HANDLE};
    XrAction triggerAction{XR_NULL_HANDLE};
    XrAction gripAction{XR_NULL_HANDLE};

    // only used in non-default interaction profiles
    XrAction thumbStickAction{XR_NULL_HANDLE};
    XrAction primaryAction{XR_NULL_HANDLE};
    XrAction secondaryAction{XR_NULL_HANDLE};

    XrSpace leftHandSpace{XR_NULL_HANDLE}, rightHandSpace{XR_NULL_HANDLE};
};
}    // namespace XR
}    // namespace XRLib
