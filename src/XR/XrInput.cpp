#include "XrInput.h"

namespace XRLib {
namespace XR {
XrInput::XrInput(std::shared_ptr<XrCore> core) : core{core} {
    XrResult result;
    XrActionSetCreateInfo actionSetCreateInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
    std::strcpy(actionSetCreateInfo.actionSetName, "xrlib_action_set");
    std::strcpy(actionSetCreateInfo.localizedActionSetName, "XRLib Action Set");
    if ((result = xrCreateActionSet(this->core->GetXRInstance(), &actionSetCreateInfo, &actionSet)) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to create openxr action set");
    }
    LOGGER(LOGGER::DEBUG) << "XRLib Action Set created";

    XrActionCreateInfo actionCI{XR_TYPE_ACTION_CREATE_INFO};
    actionCI.actionType = XR_ACTION_TYPE_POSE_INPUT;
    strcpy(actionCI.actionName, "controller_pose");
    strcpy(actionCI.localizedActionName, "Controller Pose");
    actionCI.countSubactionPaths = 2;
    XrPath subactionPaths[2] = {XrUtil::CreateXrPath(this->core->GetXRInstance(), "/user/hand/left"),
                                XrUtil::CreateXrPath(this->core->GetXRInstance(), "/user/hand/right")};
    actionCI.subactionPaths = subactionPaths;

    if ((result = xrCreateAction(actionSet, &actionCI, &controllerPoseAction)) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to create openxr pose action");
    }
    LOGGER(LOGGER::DEBUG) << "Controller pose actions created";

    std::vector<XrActionSuggestedBinding> suggestedBindings = {
        {
            controllerPoseAction,
            XrUtil::CreateXrPath(core->GetXRInstance(), "/user/hand/left/input/grip/pose"),
        },
        {
            controllerPoseAction,
            XrUtil::CreateXrPath(core->GetXRInstance(), "/user/hand/right/input/grip/pose"),
        }};

    XrInteractionProfileSuggestedBinding suggestedBindingInfo{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
    suggestedBindingInfo.interactionProfile =
        XrUtil::CreateXrPath(core->GetXRInstance(), suggestedInteractionProfile.c_str());
    suggestedBindingInfo.suggestedBindings = suggestedBindings.data();
    suggestedBindingInfo.countSuggestedBindings = static_cast<uint32_t>(suggestedBindings.size());

    result = xrSuggestInteractionProfileBindings(core->GetXRInstance(), &suggestedBindingInfo);
    if (result != XR_SUCCESS) {
        Util::ErrorPopup("Failed to suggest interaction profile bindings");
    }

    LOGGER(LOGGER::DEBUG) << "Using suggested interaction profile" << suggestedInteractionProfile;

    XrActionSpaceCreateInfo spaceInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
    spaceInfo.action = controllerPoseAction;
    spaceInfo.poseInActionSpace.orientation.w = 1.0f;

    spaceInfo.subactionPath = XrUtil::CreateXrPath(this->core->GetXRInstance(), "/user/hand/left");
    result = xrCreateActionSpace(this->core->GetXRSession(), &spaceInfo, &leftHandSpace);
    if (result != XR_SUCCESS) {
        Util::ErrorPopup("Failed to create left hand action space");
    }

    spaceInfo.subactionPath = XrUtil::CreateXrPath(this->core->GetXRInstance(), "/user/hand/right");
    result = xrCreateActionSpace(this->core->GetXRSession(), &spaceInfo, &rightHandSpace);
    if (result != XR_SUCCESS) {
        Util::ErrorPopup("Failed to create right hand action space");
    }

    XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
    attachInfo.actionSets = &actionSet;
    attachInfo.countActionSets = 1;

    result = xrAttachSessionActionSets(core->GetXRSession(), &attachInfo);
    if (result != XR_SUCCESS) {
        Util::ErrorPopup("Failed to attach action sets to the session");
    }
}

void XrInput::UpdateInput() {
    XrActiveActionSet activeActionSet{};
    activeActionSet.actionSet = actionSet;
    activeActionSet.subactionPath = XR_NULL_PATH;

    XrActionsSyncInfo syncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
    syncInfo.activeActionSets = &activeActionSet;
    syncInfo.countActiveActionSets = 1;

    xrSyncActions(core->GetXRSession(), &syncInfo);

    XrSpaceLocation spaceLocation{XR_TYPE_SPACE_LOCATION};

    if (xrLocateSpace(leftHandSpace, core->GetXrSpace(), core->GetXrFrameState().predictedDisplayTime,
                      &spaceLocation) == XR_SUCCESS) {
        if (spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) {
            EventSystem::TriggerEvent(Events::XRLIB_EVENT_LEFT_CONTROLLER_POSITION,
                                      Transform{LibMath::XrPoseToMatrix(spaceLocation.pose)});
        }
    }

    if (xrLocateSpace(rightHandSpace, core->GetXrSpace(), core->GetXrFrameState().predictedDisplayTime,
                      &spaceLocation) == XR_SUCCESS) {
        if (spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) {
            EventSystem::TriggerEvent(Events::XRLIB_EVENT_RIGHT_CONTROLLER_POSITION,
                                      Transform{LibMath::XrPoseToMatrix(spaceLocation.pose)});
        }
    }
}

}    // namespace XR
}    // namespace XRLib
