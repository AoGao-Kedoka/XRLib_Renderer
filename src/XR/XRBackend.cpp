#include "XRBackend.h"

XRBackend::XRBackend(std::shared_ptr<Info> info, std::shared_ptr<VkCore> core,
                     std::shared_ptr<XrCore> xrCore)
    : info{info}, vkCore{core}, xrCore{xrCore} {
    try {
        for (const auto layer : apiLayers) {
            bool res = XrUtil::XrCheckLayerSupport(layer.c_str());
            if (res) {
                activeAPILayers.push_back(layer.c_str());
            }
        }

        CreateXrInstance();
        GetSystemID();
        LogOpenXRRuntimeProperties();

    } catch (const std::runtime_error& e) {
        LOGGER(LOGGER::WARNING) << "Falling back to normal mode";
        xrCore->SetXRValid(false);
    }
}

XRBackend::~XRBackend() {
    if (!xrCore || !info) {
        return;
    }

    if (xrCore->IsXRValid())
        EndSession();

    if (xrDebugUtilsMessenger != XR_NULL_HANDLE) {
        PFN_xrDestroyDebugUtilsMessengerEXT xrDestroyDebugUtilsMessengerEXT =
            XrUtil::XrGetXRFunction<PFN_xrDestroyDebugUtilsMessengerEXT>(
                xrCore->GetXRInstance(), "xrDestroyDebugUtilsMessengerEXT");
        xrDestroyDebugUtilsMessengerEXT(xrDebugUtilsMessenger);
    }
}

void XRBackend::Prepare() {
    CreateXrSession();
    CreateXrSwapchain();
    PrepareXrSwapchainImages();
    BeginSession();
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

    if (std::strlen(info->applicationName.c_str()) >
        XR_MAX_APPLICATION_NAME_SIZE) {
        LOGGER(LOGGER::WARNING)
            << "Application name longer than the size allowed";
    }

    XrInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.createFlags = 0;
    std::memcpy(instanceCreateInfo.applicationInfo.applicationName,
                info->applicationName.c_str(),
                XR_MAX_APPLICATION_NAME_SIZE - 1);
    instanceCreateInfo.applicationInfo.applicationVersion = XR_MAKE_VERSION(
        info->majorVersion, info->minorVersion, info->patchVersion);
    std::memcpy(instanceCreateInfo.applicationInfo.engineName,
                info->applicationName.c_str(),
                XR_MAX_APPLICATION_NAME_SIZE - 1);
    instanceCreateInfo.applicationInfo.engineVersion = XR_MAKE_VERSION(
        info->majorVersion, info->minorVersion, info->patchVersion);
    instanceCreateInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
    instanceCreateInfo.enabledApiLayerCount =
        static_cast<uint32_t>(activeAPILayers.size());
    instanceCreateInfo.enabledApiLayerNames = activeAPILayers.data();
    instanceCreateInfo.enabledExtensionCount =
        static_cast<uint32_t>(activeInstanceExtensions.size());
    instanceCreateInfo.enabledExtensionNames = activeInstanceExtensions.data();
    XrDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
    if (info->validationLayer) {
        debugUtilsMessengerCreateInfo.type =
            XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsMessengerCreateInfo.messageTypes =
            XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
        debugUtilsMessengerCreateInfo.messageSeverities =
            XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilsMessengerCreateInfo.userCallback = XrUtil::XrDebugCallback;
        instanceCreateInfo.next = &debugUtilsMessengerCreateInfo;
    }

    XrResult result =
        xrCreateInstance(&instanceCreateInfo, &xrCore->GetXRInstance());

    if (result != XR_SUCCESS) {

        Util::ErrorPopup(
            "Failed to create XR instance, no OpenXR runtime set.");
    }

    if (info->validationLayer) {
        PFN_xrCreateDebugUtilsMessengerEXT xrCreateDebugUtilsMessengerEXT =
            XrUtil::XrGetXRFunction<PFN_xrCreateDebugUtilsMessengerEXT>(
                xrCore->GetXRInstance(), "xrCreateDebugUtilsMessengerEXT");
        if (xrCreateDebugUtilsMessengerEXT(
                xrCore->GetXRInstance(), &debugUtilsMessengerCreateInfo,
                &xrDebugUtilsMessenger) != XR_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to create debug messenger";
            info->validationLayer = false;
        }
    }
}

void XRBackend::GetSystemID() {
    XrSystemGetInfo systemGetInfo{};
    systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
    systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrResult result = xrGetSystem(xrCore->GetXRInstance(), &systemGetInfo,
                                  &xrCore->GetSystemID());
    if (result != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get system id, HMD may not connected.");
    }
}

void XRBackend::CreateXrSession() {
    auto xrGetVulkanGraphicsRequirementsKHR =
        XrUtil::XrGetXRFunction<PFN_xrGetVulkanGraphicsRequirementsKHR>(
            xrCore->GetXRInstance(), "xrGetVulkanGraphicsRequirementsKHR");
    auto result = xrGetVulkanGraphicsRequirementsKHR(
        xrCore->GetXRInstance(), xrCore->GetSystemID(),
        &xrCore->GetGraphicsRequirements());

    XrGraphicsBindingVulkanKHR graphicsBinding{};
    graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
    graphicsBinding.instance = vkCore->GetRenderInstance();
    graphicsBinding.device = vkCore->GetRenderDevice();
    graphicsBinding.queueFamilyIndex = vkCore->GetGraphicsQueueFamilyIndex();
    graphicsBinding.physicalDevice = vkCore->GetRenderPhysicalDevice();
    graphicsBinding.queueIndex = 0;

    XrSessionCreateInfo sessionCreateInfo{};
    sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO;
    sessionCreateInfo.systemId = xrCore->GetSystemID();
    sessionCreateInfo.next = &graphicsBinding;
    if (xrCreateSession(xrCore->GetXRInstance(), &sessionCreateInfo,
                        &xrCore->GetXRSession()) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to create session");
    }
}

void XRBackend::CreateXrSwapchain() {
    uint32_t swapchainFormatCount;
    if (xrEnumerateSwapchainFormats(xrCore->GetXRSession(), 0,
                                    &swapchainFormatCount,
                                    nullptr) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get swapchain formats");
    }
    std::vector<int64_t> swapchainFormats(swapchainFormatCount);
    if (xrEnumerateSwapchainFormats(xrCore->GetXRSession(),
                                    swapchainFormatCount, &swapchainFormatCount,
                                    swapchainFormats.data()) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get swapchain formats");
    }

    vkCore->SetStereoSwapchainImageFormat(
        static_cast<VkFormat>(swapchainFormats.at(0)));

    uint32_t viewCount;
    if (xrEnumerateViewConfigurationViews(
            xrCore->GetXRInstance(), xrCore->GetSystemID(),
            XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount,
            nullptr) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get view configuration views");
    }

    xrCore->GetXRViewConfigurationView().resize(viewCount);
    for (XrViewConfigurationView& viewInfo :
         xrCore->GetXRViewConfigurationView()) {
        viewInfo.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
        viewInfo.next = nullptr;
    }
    if (xrEnumerateViewConfigurationViews(
            xrCore->GetXRInstance(), xrCore->GetSystemID(),
            XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount, &viewCount,
            xrCore->GetXRViewConfigurationView().data()) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get view configuration views");
    }

    XrResult result;
    XrSwapchainCreateInfo swapchainCreateInfo{};
    swapchainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
    swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.format =
        static_cast<uint32_t>(vkCore->GetStereoSwapchainImageFormat());
    swapchainCreateInfo.sampleCount =
        xrCore->GetXRViewConfigurationView()[0].recommendedSwapchainSampleCount;
    swapchainCreateInfo.width =
        xrCore->GetXRViewConfigurationView()[0].recommendedImageRectWidth;
    swapchainCreateInfo.height =
        xrCore->GetXRViewConfigurationView()[0].recommendedImageRectHeight;
    swapchainCreateInfo.faceCount = 1;
    swapchainCreateInfo.arraySize = viewCount;
    swapchainCreateInfo.mipCount = 1;
    if ((result =
             xrCreateSwapchain(xrCore->GetXRSession(), &swapchainCreateInfo,
                               &xrCore->GetXrSwapchain())) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to create swapchain");
    }
}

void XRBackend::PrepareXrSwapchainImages() {
    XrResult result;

    uint32_t swapchainImageCount;
    if ((result = xrEnumerateSwapchainImages(xrCore->GetXrSwapchain(), 0,
                                             &swapchainImageCount, nullptr)) !=
        XR_SUCCESS) {
        Util::ErrorPopup("Failed to enumerate swapchain images");
    }

    xrCore->GetSwapchainImages().resize(swapchainImageCount);
    for (XrSwapchainImageVulkanKHR& swapchainImage :
         xrCore->GetSwapchainImages()) {
        swapchainImage.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
    }

    XrSwapchainImageBaseHeader* data =
        reinterpret_cast<XrSwapchainImageBaseHeader*>(
            xrCore->GetSwapchainImages().data());
    if ((result = xrEnumerateSwapchainImages(
             xrCore->GetXrSwapchain(), xrCore->GetSwapchainImages().size(),
             &swapchainImageCount, data)) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get swapchain images");
    }

    xrCore->GetCompositionLayerProjectionViews().resize(
        xrCore->GetXRViewConfigurationView().size());
    for (size_t i = 0; i < xrCore->GetCompositionLayerProjectionViews().size();
         ++i) {
        XrCompositionLayerProjectionView& projectionView =
            xrCore->GetCompositionLayerProjectionViews()[i];
        projectionView.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
        projectionView.next = nullptr;

        projectionView.subImage.swapchain = xrCore->GetXrSwapchain();
        projectionView.subImage.imageArrayIndex = i;
        projectionView.subImage.imageRect.offset = {0, 0};
        projectionView.subImage.imageRect.extent = {
            static_cast<int32_t>(xrCore->GetXRViewConfigurationView()[i]
                                     .recommendedImageRectWidth),
            static_cast<int32_t>(xrCore->GetXRViewConfigurationView()[i]
                                     .recommendedImageRectHeight),
        };
    }
}
XrResult XRBackend::StartFrame(uint32_t& imageIndex) {
    XrResult result;

    PollXREvents();
    XrFrameWaitInfo frameWaitInfo{XR_TYPE_FRAME_WAIT_INFO};
    xrWaitFrame(xrCore->GetXRSession(), &frameWaitInfo,
                &xrCore->GetXrFrameState());

    XrFrameBeginInfo frameBeginInfo{XR_TYPE_FRAME_BEGIN_INFO};
    if ((result = xrBeginFrame(xrCore->GetXRSession(), &frameBeginInfo)) !=
        XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to begin frame";
        return result;
    }

    XrSwapchainImageAcquireInfo swapchainImageAcquireInfo{
        XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
    if ((result = xrAcquireSwapchainImage(xrCore->GetXrSwapchain(),
                                          &swapchainImageAcquireInfo,
                                          &imageIndex)) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to get acquire swapchain";
        return result;
    }

    XrSwapchainImageWaitInfo swapchainImageWaitInfo{
        XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
    swapchainImageWaitInfo.timeout = XR_INFINITE_DURATION;
    if ((result = xrWaitSwapchainImage(xrCore->GetXrSwapchain(),
                                       &swapchainImageWaitInfo)) !=
        XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to wait swapchain image";
        return result;
    }
    return XR_SUCCESS;
}

XrResult XRBackend::EndFrame(uint32_t& imageIndex) {
    XrResult result;
    XrSwapchainImageReleaseInfo swapchainImageReleaseInfo{
        XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
    if ((result = xrReleaseSwapchainImage(xrCore->GetXrSwapchain(),
                                          &swapchainImageReleaseInfo)) !=
        XR_SUCCESS) {
        return result;
    }
    XrCompositionLayerProjection compositionLayerProjection{
        XR_TYPE_COMPOSITION_LAYER_PROJECTION};
    compositionLayerProjection.space = xrCore->GetXrSpace();
    compositionLayerProjection.viewCount =
        xrCore->GetXRViewConfigurationView().size();
    compositionLayerProjection.views =
        xrCore->GetCompositionLayerProjectionViews().data();

    //TODO end view state

    XrFrameEndInfo frameEndInfo{XR_TYPE_FRAME_END_INFO};
    frameEndInfo.displayTime = xrCore->GetXrFrameState().predictedDisplayTime;
    frameEndInfo.layerCount = 1;
    frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    if ((result = xrEndFrame(xrCore->GetXRSession(), &frameEndInfo)) !=
        XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to end frame";
        return result;
    }

    return XR_SUCCESS;
}

void XRBackend::PollXREvents() {
    XrEventDataBuffer buffer{XR_TYPE_EVENT_DATA_BUFFER};
    //TODO
}

void XRBackend::BeginSession() {
    XrResult result;
    XrSessionBeginInfo sessionBeginInfo{};
    sessionBeginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
    sessionBeginInfo.primaryViewConfigurationType =
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    if ((result = xrBeginSession(xrCore->GetXRSession(), &sessionBeginInfo))) {
        Util::ErrorPopup("Failed to begin xr session");
    }
}

void XRBackend::EndSession() {
    XrResult result;
    if (result = xrEndSession(xrCore->GetXRSession())) {
        Util::ErrorPopup("Failed to end xr session");
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
