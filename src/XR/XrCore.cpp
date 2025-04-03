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

std::vector<const char*> XrCore::VkAdditionalInstanceExts2() {
#if defined(_WIN32) || defined(_WIN64)
    return {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
            "VK_KHR_win32_surface"};
#elif defined(__linux__)
    return {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
            "VK_KHR_xcb_surface"};
#else
    return {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};
#endif
}

// deprecated
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

std::vector<const char*> XrCore::VkAdditionalDeviceExts2() {
    return {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME};
}

// deprecated
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

void XrCore::VkSetPhysicalDevice2(VkInstance instance, VkPhysicalDevice* physicalDevice) {
    XrVulkanGraphicsDeviceGetInfoKHR deviceGetInfo{XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR};
    deviceGetInfo.systemId = GetSystemID();
    deviceGetInfo.vulkanInstance = instance;

    auto xrGetGraphicsDevice2KHR =
        XrUtil::XrGetXRFunction<PFN_xrGetVulkanGraphicsDevice2KHR>(GetXRInstance(), "xrGetVulkanGraphicsDevice2KHR");
    XrResult result = xrGetGraphicsDevice2KHR(GetXRInstance(), &deviceGetInfo, physicalDevice);
    if (result != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get vulkan graphics device");
    }
}
}    // namespace XR
}    // namespace XRLib
