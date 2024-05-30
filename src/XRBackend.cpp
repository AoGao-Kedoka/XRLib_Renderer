#include "XRBackend.h"
#include "NMB.h"
#include "Util.h"

XRBackend::XRBackend(Info& info, VkCore& vkCore, XrCore& xrCore)
    : info{&info}, vkCore{&vkCore}, xrCore{&xrCore} {
    CreateXrInstance();
    GetSystemID();
    if (xrCore.IsXRValid()) {
        LogOpenXRRuntimeProperties();
    }
}

XRBackend::~XRBackend() {
    if (!xrCore|| !info) {
        return;
    }

    Util::XrSafeClean(xrDestroyInstance, xrCore->GetXRInstance());
    Util::XrSafeClean(xrDestroySession, xrCore->GetXRSession());
}

void XRBackend::XrCreateSwapcahin() {
    uint32_t swapchainFormatCount;
    if (xrEnumerateSwapchainFormats(xrCore->GetXRSession(), 0,
                                    &swapchainFormatCount,
                                    nullptr) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to get swapchain formats";
        exit(-1);
    }
    std::vector<int64_t> swpchainFormats(swapchainFormatCount);
    if (xrEnumerateSwapchainFormats(xrCore->GetXRSession(), swapchainFormatCount,
                                    &swapchainFormatCount,
                                    swpchainFormats.data()) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to get swapchain formats";
        exit(-1);
    }

    uint32_t viewCount;
    if (xrEnumerateViewConfigurationViews(
            xrCore->GetXRInstance(), xrCore->GetSystemID(),
            XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount,
            nullptr) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to get view configuration views";
        exit(-1);
    }

    xrCore->GetXRViewConfigurationView().resize(viewCount);
    if (xrEnumerateViewConfigurationViews(
            xrCore->GetXRInstance(), xrCore->GetSystemID(),
            XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount, &viewCount,
            xrCore->GetXRViewConfigurationView().data()) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to get view configuration views";
        exit(-1);
    }
    //TODO
}

void XRBackend::CreateXrInstance() {

    if (info->validationLayer) {
        activeInstanceExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    uint32_t apiLayerCount = 0;
    std::vector<XrApiLayerProperties> apiLayerProperties;
    if ((xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr) !=
         XR_SUCCESS)) {
        LOGGER(LOGGER::ERR) << "Failed to enumerate api layer properties";
    }
    apiLayerProperties.resize(apiLayerCount, {XR_TYPE_API_LAYER_PROPERTIES});
    if (xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount,
                                      apiLayerProperties.data())) {
        LOGGER(LOGGER::ERR) << "Failed to enumerate api layer properties";
    }

    // Check the requested API layers against the ones from the OpenXR. If found add it to the Active API Layers.
    for (auto& requestLayer : apiLayers) {
        for (auto& layerProperty : apiLayerProperties) {
            if (strcmp(requestLayer.c_str(), layerProperty.layerName) != 0) {
                continue;
            } else {
                activeAPILayers.push_back(requestLayer.c_str());
                break;
            }
        }
    }

    uint32_t extensionCount = 0;
    std::vector<XrExtensionProperties> extensionProperties;
    if (xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount,
                                               nullptr) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR)
            << "Failed to enumerate InstanceExtensionProperties.";
    }
    extensionProperties.resize(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
    if (xrEnumerateInstanceExtensionProperties(
            nullptr, extensionCount, &extensionCount,
            extensionProperties.data()) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR)
            << "Failed to enumerate InstanceExtensionProperties.";
    }
    for (auto& requestedInstanceExtension : instanceExtensions) {
        bool found = false;
        for (auto& extensionProperty : extensionProperties) {
            if (strcmp(requestedInstanceExtension.c_str(),
                       extensionProperty.extensionName) != 0) {
                continue;
            } else {
                activeInstanceExtensions.push_back(
                    requestedInstanceExtension.c_str());
                found = true;
                break;
            }
        }
        if (!found) {
            LOGGER(LOGGER::ERR) << "Failed to find OpenXR instance extension: "
                                << requestedInstanceExtension;
        }
    }

    XrInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.createFlags = 0;
    strcpy(instanceCreateInfo.applicationInfo.applicationName,
           info->applicationName.c_str());
    instanceCreateInfo.applicationInfo.applicationVersion = XR_MAKE_VERSION(
        info->majorVersion, info->minorVersion, info->patchVersion);
    strcpy(instanceCreateInfo.applicationInfo.engineName,
           info->applicationName.c_str());
    instanceCreateInfo.applicationInfo.engineVersion = XR_MAKE_VERSION(
        info->majorVersion, info->minorVersion, info->patchVersion);
    instanceCreateInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
    instanceCreateInfo.enabledApiLayerCount =
        static_cast<uint32_t>(activeAPILayers.size());
    instanceCreateInfo.enabledApiLayerNames = activeAPILayers.data();
    instanceCreateInfo.enabledExtensionCount =
        static_cast<uint32_t>(activeInstanceExtensions.size());
    instanceCreateInfo.enabledExtensionNames = activeInstanceExtensions.data();

    XrResult result =
        xrCreateInstance(&instanceCreateInfo, &xrCore->GetXRInstance());

    if (result != XR_SUCCESS) {
        std::string message{
            "Failed to create XR instance, no OpenXR runtime set."};
        LOGGER(LOGGER::ERR) << message;
        NMB::show("Error", message.c_str(), NMB::Icon::ICON_ERROR);
        exit(-1);
    }
}

void XRBackend::GetSystemID() {
    XrSystemGetInfo systemGetInfo{};
    systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
    systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrResult result = xrGetSystem(xrCore->GetXRInstance(), &systemGetInfo,
                                  &xrCore->GetSystemID());
    if (result != XR_SUCCESS) {
        std::string message{
            "Failed to get system id, HMD may not connected. Fall back to "
            "normal rendering mode"};
        LOGGER(LOGGER::WARNING) << message;
        NMB::show("Error", message.c_str(), NMB::Icon::ICON_ERROR);
        xrCore->SetXRValid(false);
    }
}

void XRBackend::CreateXrSession() {
    XrGraphicsBindingVulkanKHR graphicsBinding;
    graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
    graphicsBinding.instance = vkCore->GetRenderInstance();
    graphicsBinding.device = vkCore->GetRenderDevice();
    graphicsBinding.queueFamilyIndex = vkCore->GetGraphicsQueueFamilyIndex();
    graphicsBinding.physicalDevice = vkCore->GetRenderPhysicalDevice();
    graphicsBinding.queueIndex = 0;

    XrSessionCreateInfo sessionCreateInfo{};
    sessionCreateInfo.createFlags = XR_TYPE_SESSION_CREATE_INFO;
    sessionCreateInfo.systemId = xrCore->GetSystemID();
    sessionCreateInfo.next = &graphicsBinding;
    if (xrCreateSession(xrCore->GetXRInstance(), &sessionCreateInfo,
                        &xrCore->GetXRSession()) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create session";
        exit(-1);
    }
}

void XRBackend::LogOpenXRRuntimeProperties() const {

    if (xrCore->GetXRInstance() == XR_NULL_HANDLE) {
        LOGGER(LOGGER::ERR) << "XR Instance is null";
    }

    XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
    if (xrGetInstanceProperties(xrCore->GetXRInstance(), &instanceProperties) !=
        XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to get instance properties";
    } else {
        LOGGER(LOGGER::INFO)
            << "Using OpenXR Runtime: " << instanceProperties.runtimeName
            << " - " << XR_VERSION_MAJOR(instanceProperties.runtimeVersion)
            << XR_VERSION_MINOR(instanceProperties.runtimeVersion)
            << XR_VERSION_PATCH(instanceProperties.runtimeVersion);
    }
}

void XRBackend::LogOpenXRSystemProperties() const {
    XrSystemProperties systemProperties{XR_TYPE_SYSTEM_PROPERTIES};
    if (xrGetSystemProperties(xrCore->GetXRInstance(), xrCore->GetSystemID(),
                              &systemProperties) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to get system properties";
    } else {
        //TODO
    }
}
