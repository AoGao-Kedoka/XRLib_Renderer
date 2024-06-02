#include "XrCore.h"

XrCore::~XrCore() {
    Util::XrSafeClean(xrDestroySpace, xrSceneSpace);
    Util::XrSafeClean(xrDestroyInstance, xrInstance);
    Util::XrSafeClean(xrDestroySession, xrSession);
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
    referenceSpaceCreateInfo.poseInReferenceSpace = LAMath::Identity();
    if (xrCreateReferenceSpace(xrSession, &referenceSpaceCreateInfo,
                               &xrSceneSpace) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create play space";
        exit(-1);
    }
}
