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
    referenceSpaceCreateInfo.poseInReferenceSpace = MathUtil::XrIdentity();
    if (xrCreateReferenceSpace(xrSession, &referenceSpaceCreateInfo, &xrSceneSpace) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to create play space");
    }
}

std::vector<const char*> XrCore::VkAdditionalInstanceExts() {
    std::vector<const char*> additionalInstanceExts;
    if (!IsXRValid()) {
        return additionalInstanceExts;
    }
    auto xrGetVulkanInstanceExtensionsKHR = XrUtil::XrGetXRFunction<PFN_xrGetVulkanInstanceExtensionsKHR>(
        GetXRInstance(), "xrGetVulkanInstanceExtensionsKHR");

    uint32_t xrVulkanInstanceExtensionsCount;
    if (xrGetVulkanInstanceExtensionsKHR(GetXRInstance(), GetSystemID(), 0, &xrVulkanInstanceExtensionsCount,
                                         nullptr) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get vulkan instance extension count");
    }

    std::string buffer(xrVulkanInstanceExtensionsCount, ' ');
    if (xrGetVulkanInstanceExtensionsKHR(GetXRInstance(), GetSystemID(), xrVulkanInstanceExtensionsCount,
                                         &xrVulkanInstanceExtensionsCount, buffer.data()) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get vulkan instance extension");
    }

    std::vector<const char*> xrInstanceExtensions = Util::SplitStringToCharPtr(buffer);
    for (auto extension : xrInstanceExtensions) {
        additionalInstanceExts.push_back(extension);
    }

    additionalInstanceExts.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    return additionalInstanceExts;
}

std::vector<const char*> XrCore::VkAdditionalDeviceExts() {
    std::vector<const char*> additionalDeviceExts;
    if (!IsXRValid()) {
        return additionalDeviceExts;
    }
    auto xrGetVulkanDeviceExtensionsKHR = XRLib::XR::XrUtil::XrGetXRFunction<PFN_xrGetVulkanDeviceExtensionsKHR>(
        GetXRInstance(), "xrGetVulkanDeviceExtensionsKHR");

    uint32_t deviceExtensionsCount;
    if (xrGetVulkanDeviceExtensionsKHR(GetXRInstance(), GetSystemID(), 0, &deviceExtensionsCount, nullptr) !=
        XR_SUCCESS) {
        Util::ErrorPopup("Failed to get vulkan device extensions");
    }
    std::string buffer(deviceExtensionsCount, ' ');
    if (xrGetVulkanDeviceExtensionsKHR(GetXRInstance(), GetSystemID(), deviceExtensionsCount, &deviceExtensionsCount,
                                       buffer.data())) {
        Util::ErrorPopup("Failed to get vulkan device extensions");
    }
    additionalDeviceExts = Util::SplitStringToCharPtr(buffer);

    additionalDeviceExts.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    return additionalDeviceExts;
}

void XrCore::VkSetPhysicalDevice(VkInstance instance, VkPhysicalDevice* physicalDevice) {
    auto xrGetVulkanGraphicsDeviceKHR =
        XrUtil::XrGetXRFunction<PFN_xrGetVulkanGraphicsDeviceKHR>(GetXRInstance(), "xrGetVulkanGraphicsDeviceKHR");

    if (xrGetVulkanGraphicsDeviceKHR(GetXRInstance(), GetSystemID(), instance, physicalDevice) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get vulkan graphics device");
    }
}
}    // namespace XR
}    // namespace XRLib
