#include "XRBackend.h"
#include "NMB.h"

XRBackend::XRBackend(Info& info, std::shared_ptr<RenderBackend> renderBackend)
    : info{&info}, renderBackend{std::move(renderBackend)} {
    CreateXrInstance();
    GetSystemID();
    GetFunctionExtensions();
    LogOpenXRRuntimeProperties();
}

XRBackend::~XRBackend() {
    Cleanup();
}

void XRBackend::Cleanup() const {

    if (xrInstance != XR_NULL_HANDLE) {
        xrDestroyInstance(xrInstance);
    }

    if (xrSession != XR_NULL_HANDLE) {
        xrDestroySession(xrSession);
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

    XrResult result = xrCreateInstance(&instanceCreateInfo, &xrInstance);

    if (result != XR_SUCCESS) {
        std::string message{
            "Failed to create XR instance, no OpenXR runtime set."};
        LOGGER(LOGGER::ERR) << message;
        NMB::show("Error", message.c_str(), NMB::Icon::ICON_ERROR);
        Cleanup();
        exit(-1);
    }
}

void XRBackend::GetSystemID() {
    XrSystemGetInfo systemGetInfo{};
    systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
    systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrResult result = xrGetSystem(xrInstance, &systemGetInfo, &xrSystemID);
    if (result != XR_SUCCESS) {
        std::string message{"Failed to get system id, HMD may not connected."};
        LOGGER(LOGGER::ERR) << message;
        NMB::show("Error", message.c_str(), NMB::Icon::ICON_ERROR);
        Cleanup();
        exit(-1);
    }
}

void XRBackend::GetFunctionExtensions() const {
    LoadXRExtensionFunctions();
    uint32_t xrVulkanInstanceExtensionsCount;
    if (xrGetVulkanInstanceExtensionsKHR(xrInstance, xrSystemID, 0,
        &xrVulkanInstanceExtensionsCount,
        nullptr) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to get vulkan instance extension count";
        Cleanup();
        exit(-1);
    }

    std::string buffer(xrVulkanInstanceExtensionsCount, ' ');
    if (xrGetVulkanInstanceExtensionsKHR(xrInstance, xrSystemID, 0,
                                         &xrVulkanInstanceExtensionsCount, buffer.data()) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to get vulkan instance extension";
        Cleanup();
        exit(-1);
    }

    renderBackend->GetVulkanInstanceExtensions().push_back(buffer.c_str());
    renderBackend->CreateVulkanInstance();
}

void XRBackend::CreateXrSession() {

    XrGraphicsBindingVulkanKHR graphicsBinding;
    graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
    graphicsBinding.instance = renderBackend->GetRenderInstance();
    graphicsBinding.device = renderBackend->GetRenderDevice();
    graphicsBinding.queueFamilyIndex = renderBackend->GetQueueFamilyIndex();
    graphicsBinding.physicalDevice = renderBackend->GetRenderPhysicalDevice();
    graphicsBinding.queueIndex = 0;

    XrSessionCreateInfo sessionCreateInfo{};
    sessionCreateInfo.createFlags = XR_TYPE_SESSION_CREATE_INFO;
    sessionCreateInfo.systemId = xrSystemID;
    sessionCreateInfo.next = &graphicsBinding;
    if (xrCreateSession(xrInstance, &sessionCreateInfo, &xrSession) !=
        XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create session";
        Cleanup();
        exit(-1);
    }
}

void XRBackend::LogOpenXRRuntimeProperties() const {

    if (xrInstance == XR_NULL_HANDLE) {
        LOGGER(LOGGER::ERR) << "XR Instance is null";
    }

    XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
    xrGetInstanceProperties(xrInstance, &instanceProperties);
    LOGGER(LOGGER::INFO) << "Using OpenXR Runtime: "
                         << instanceProperties.runtimeName << " - "
                         << XR_VERSION_MAJOR(instanceProperties.runtimeVersion)
                         << XR_VERSION_MINOR(instanceProperties.runtimeVersion)
                         << XR_VERSION_PATCH(instanceProperties.runtimeVersion);
}

void XRBackend::LoadXRExtensionFunctions() const {
    const std::vector<std::pair<PFN_xrVoidFunction*, std::string>> functionPairs{
        {reinterpret_cast<PFN_xrVoidFunction*>(xrGetVulkanInstanceExtensionsKHR),
         "xrGetVulkanInstanceExtensionsKHR"},
        {reinterpret_cast<PFN_xrVoidFunction*>(xrGetVulkanGraphicsDeviceKHR),
         "xrGetVulkanGraphicsDeviceKHR"},
        {reinterpret_cast<PFN_xrVoidFunction*>(xrGetVulkanDeviceExtensionsKHR),
         "xrGetVulkanDeviceExtensionsKHR"},
        {reinterpret_cast<PFN_xrVoidFunction*>(xrGetVulkanGraphicsRequirementsKHR),
         "xrGetVulkanGraphicsRequirementsKHR"},
    };

    for (const auto& [fst, snd] : functionPairs) {
        if (xrGetInstanceProcAddr(xrInstance, snd.c_str(),
                                  fst) != XR_SUCCESS) {
            LOGGER(LOGGER::WARNING)
                << "OpenXR extension" << snd << "not supported";
        }
    }
}
