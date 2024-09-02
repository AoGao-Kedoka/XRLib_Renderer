#include "XrCore.h"

namespace XRLib {
namespace XR {
XrCore::~XrCore() {
    XrUtil::XrSafeClean(xrDestroySpace, xrSceneSpace);
    XrUtil::XrSafeClean(xrDestroyInstance, xrInstance);
    XrUtil::XrSafeClean(xrDestroySession, xrSession);
}

void XrCore::CreatePlaySpace() {
    if (xrSession == VK_NULL_HANDLE) {
        LOGGER(LOGGER::WARNING) << "XR Session not initialized";
        return;
    }
    const XrReferenceSpaceType spaceType{XR_REFERENCE_SPACE_TYPE_STAGE};
    XrReferenceSpaceCreateInfo referenceSpaceCreateInfo{};
    referenceSpaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
    referenceSpaceCreateInfo.referenceSpaceType = spaceType;
    referenceSpaceCreateInfo.poseInReferenceSpace = LibMath::XrIdentity();
    if (xrCreateReferenceSpace(xrSession, &referenceSpaceCreateInfo,
                               &xrSceneSpace) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to create play space");
    }
}
}    // namespace XR
}    // namespace XRLib
