#include "XRBackend.h"
#include "NMB.h"


XRBackend::XRBackend(Info& info, Core& core) : info{&info}, core{&core} {
    CreateXrInstance();
    GetSystemID();
    if (core.IsXRValid()) {
        LogOpenXRRuntimeProperties();
    }
}

XRBackend::~XRBackend() {
    if (core->GetXRInstance() != XR_NULL_HANDLE) {
        xrDestroyInstance(core->GetXRInstance());
    }

    if (core->GetXRSession()!= XR_NULL_HANDLE) {
        xrDestroySession(core->GetXRSession());
    }
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
                                               nullptr)) {
        LOGGER(LOGGER::ERR)
            << "Failed to enumerate InstanceExtensionProperties.";
    }
    extensionProperties.resize(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
    if (xrEnumerateInstanceExtensionProperties(nullptr, extensionCount,
                                               &extensionCount,
                                               extensionProperties.data())) {
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

    XrResult result = xrCreateInstance(&instanceCreateInfo, &core->GetXRInstance());

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
    XrResult result =
        xrGetSystem(core->GetXRInstance(), &systemGetInfo, &core->GetSystemID());
    if (result != XR_SUCCESS) {
        std::string message{"Failed to get system id, HMD may not connected. Fall back to normal rendering mode"};
        LOGGER(LOGGER::WARNING) << message;
        NMB::show("Error", message.c_str(), NMB::Icon::ICON_ERROR);
        core->SetXRValid(false);
    }
}

void XRBackend::CreateXrSession() {
    XrGraphicsBindingVulkanKHR graphicsBinding;
    graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
    graphicsBinding.instance = core->GetRenderInstance();
    graphicsBinding.device = core->GetRenderDevice();
    graphicsBinding.queueFamilyIndex = core->GetGraphicsQueueFamilyIndex();
    graphicsBinding.physicalDevice = core->GetRenderPhysicalDevice();
    graphicsBinding.queueIndex = 0;

    XrSessionCreateInfo sessionCreateInfo{};
    sessionCreateInfo.createFlags = XR_TYPE_SESSION_CREATE_INFO;
    sessionCreateInfo.systemId = core->GetSystemID();
    sessionCreateInfo.next = &graphicsBinding;
    if (xrCreateSession(core->GetXRInstance(), &sessionCreateInfo,
                        &core->GetXRSession()) !=
        XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create session";
        exit(-1);
    }
}

void XRBackend::LogOpenXRRuntimeProperties() const {

    if (core->GetXRInstance() == XR_NULL_HANDLE) {
        LOGGER(LOGGER::ERR) << "XR Instance is null";
    }

    XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
    xrGetInstanceProperties(core->GetXRInstance(), &instanceProperties);
    LOGGER(LOGGER::INFO) << "Using OpenXR Runtime: "
                         << instanceProperties.runtimeName << " - "
                         << XR_VERSION_MAJOR(instanceProperties.runtimeVersion)
                         << XR_VERSION_MINOR(instanceProperties.runtimeVersion)
                         << XR_VERSION_PATCH(instanceProperties.runtimeVersion);
}
