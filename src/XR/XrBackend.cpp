#include "XrBackend.h"

namespace XRLib {
namespace XR {
XrBackend::XrBackend(Info& info, Graphics::VkCore& core, XrCore& xrCore) : info{info}, vkCore{core}, xrCore{xrCore} {
    try {
        for (const auto layer : apiLayers) {
            bool res = XrUtil::XrCheckLayerSupport(layer.c_str());
            if (res) {
                activeAPILayers.push_back(layer.c_str());
            }
        }

        CreateXrInstance();

        // Fetch system id
        XrSystemGetInfo systemGetInfo{};
        systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
        systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
        XrResult result = xrGetSystem(xrCore.GetXRInstance(), &systemGetInfo, &xrCore.GetSystemID());
        if (result != XR_SUCCESS) {
            Util::ErrorPopup("Failed to get system id, HMD may not connected.");
        }

        // log system infomation
        XrUtil::LogXrRuntimeProperties(xrCore.GetXRInstance());
        XrUtil::LogXrSystemProperties(xrCore.GetXRInstance(), xrCore.GetSystemID());

        EventSystem::TriggerEvent(Events::XRLIB_EVENT_XRBACKEND_INIT_FINISHED);

        EventSystem::Callback<> callback = [this]() {
            Prepare();
        };

        EventSystem::RegisterListener(Events::XRLIB_EVENT_RENDERBACKEND_INIT_FINISHED, callback);
    } catch (const std::runtime_error& e) {
        LOGGER(LOGGER::WARNING) << "Falling back to normal mode";
        xrCore.SetXRValid(false);
    }
}

XrBackend::~XrBackend() {
    if (xrDebugUtilsMessenger != XR_NULL_HANDLE) {
        PFN_xrDestroyDebugUtilsMessengerEXT xrDestroyDebugUtilsMessengerEXT =
            XrUtil::XrGetXRFunction<PFN_xrDestroyDebugUtilsMessengerEXT>(xrCore.GetXRInstance(),
                                                                         "xrDestroyDebugUtilsMessengerEXT");
        xrDestroyDebugUtilsMessengerEXT(xrDebugUtilsMessenger);
    }
}

void XrBackend::Prepare() {
    if (!xrCore.IsXRValid())
        return;
    CreateXrSession();
    CreateXrSwapchain();
    PrepareXrSwapchainImages();
    input = std::move(XrInput(xrCore));
}

void XrBackend::CreateXrInstance() {
    if (info.validationLayer) {
        activeInstanceExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    uint32_t apiLayerCount = 0;
    std::vector<XrApiLayerProperties> apiLayerProperties;
    if ((xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr) != XR_SUCCESS)) {
        LOGGER(LOGGER::ERR) << "Failed to enumerate api layer properties";
    }
    apiLayerProperties.resize(apiLayerCount, {XR_TYPE_API_LAYER_PROPERTIES});
    if (xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data())) {
        LOGGER(LOGGER::ERR) << "Failed to enumerate api layer properties";
    }

    // Check the requested API layers against the ones from the OpenXR. If found
    // add it to the Active API Layers.
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
    if (xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to enumerate InstanceExtensionProperties.";
    }
    extensionProperties.resize(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
    if (xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensionProperties.data()) !=
        XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to enumerate InstanceExtensionProperties.";
    }
    for (auto& requestedInstanceExtension : instanceExtensions) {
        bool found = false;
        for (auto& extensionProperty : extensionProperties) {
            if (strcmp(requestedInstanceExtension.c_str(), extensionProperty.extensionName) != 0) {
                continue;
            } else {
                activeInstanceExtensions.push_back(requestedInstanceExtension.c_str());
                found = true;
                break;
            }
        }
        if (!found) {
            LOGGER(LOGGER::ERR) << "Failed to find OpenXR instance extension: " << requestedInstanceExtension;
        }
    }

    if (std::strlen(info.applicationName.c_str()) > XR_MAX_APPLICATION_NAME_SIZE) {
        LOGGER(LOGGER::WARNING) << "Application name longer than the size allowed";
    }

    XrInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.createFlags = 0;
    std::memcpy(instanceCreateInfo.applicationInfo.applicationName, info.applicationName.c_str(),
                XR_MAX_APPLICATION_NAME_SIZE - 1);
    instanceCreateInfo.applicationInfo.applicationVersion =
        XR_MAKE_VERSION(info.majorVersion, info.minorVersion, info.patchVersion);
    std::memcpy(instanceCreateInfo.applicationInfo.engineName, info.applicationName.c_str(),
                XR_MAX_APPLICATION_NAME_SIZE - 1);
    instanceCreateInfo.applicationInfo.engineVersion =
        XR_MAKE_VERSION(info.majorVersion, info.minorVersion, info.patchVersion);
    instanceCreateInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
    instanceCreateInfo.enabledApiLayerCount = static_cast<uint32_t>(activeAPILayers.size());
    instanceCreateInfo.enabledApiLayerNames = activeAPILayers.data();
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(activeInstanceExtensions.size());
    instanceCreateInfo.enabledExtensionNames = activeInstanceExtensions.data();
    XrDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
    if (info.validationLayer) {
        debugUtilsMessengerCreateInfo.type = XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsMessengerCreateInfo.messageTypes =
            XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
        debugUtilsMessengerCreateInfo.messageSeverities =
            XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilsMessengerCreateInfo.userCallback = XrUtil::XrDebugCallback;
        instanceCreateInfo.next = &debugUtilsMessengerCreateInfo;
    }

    XrResult result = xrCreateInstance(&instanceCreateInfo, &xrCore.GetXRInstance());

    if (result != XR_SUCCESS) {
        Util::ErrorPopup("Failed to create XR instance, no OpenXR runtime set.");
    }

    if (info.validationLayer) {
        PFN_xrCreateDebugUtilsMessengerEXT xrCreateDebugUtilsMessengerEXT =
            XrUtil::XrGetXRFunction<PFN_xrCreateDebugUtilsMessengerEXT>(xrCore.GetXRInstance(),
                                                                        "xrCreateDebugUtilsMessengerEXT");
        if (xrCreateDebugUtilsMessengerEXT(xrCore.GetXRInstance(), &debugUtilsMessengerCreateInfo,
                                           &xrDebugUtilsMessenger) != XR_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to create debug messenger";
            info.validationLayer = false;
        }
    }
}

void XrBackend::CreateXrSession() {
    auto xrGetVulkanGraphicsRequirementsKHR = XrUtil::XrGetXRFunction<PFN_xrGetVulkanGraphicsRequirementsKHR>(
        xrCore.GetXRInstance(), "xrGetVulkanGraphicsRequirementsKHR");
    auto result = xrGetVulkanGraphicsRequirementsKHR(xrCore.GetXRInstance(), xrCore.GetSystemID(),
                                                     &xrCore.GetGraphicsRequirements());
    if (result != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to get vulkan graphics requirements";
    }

    XrGraphicsBindingVulkanKHR graphicsBinding{};
    graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
    graphicsBinding.instance = vkCore.GetRenderInstance();
    graphicsBinding.device = vkCore.GetRenderDevice();
    graphicsBinding.queueFamilyIndex = vkCore.GetGraphicsQueueFamilyIndex();
    graphicsBinding.physicalDevice = vkCore.GetRenderPhysicalDevice();
    graphicsBinding.queueIndex = 0;

    XrSessionCreateInfo sessionCreateInfo{};
    sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO;
    sessionCreateInfo.systemId = xrCore.GetSystemID();
    sessionCreateInfo.next = &graphicsBinding;
    if (xrCreateSession(xrCore.GetXRInstance(), &sessionCreateInfo, &xrCore.GetXRSession()) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to create session");
    }
}

void XrBackend::CreateXrSwapchain() {
    uint32_t swapchainFormatCount;
    if (xrEnumerateSwapchainFormats(xrCore.GetXRSession(), 0, &swapchainFormatCount, nullptr) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get swapchain formats");
    }

    xrCore.SwapchainFormats().resize(swapchainFormatCount);

    if (xrEnumerateSwapchainFormats(xrCore.GetXRSession(), swapchainFormatCount, &swapchainFormatCount,
                                    xrCore.SwapchainFormats().data()) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get swapchain formats");
    }

    if (xrEnumerateViewConfigurationViews(xrCore.GetXRInstance(), xrCore.GetSystemID(),
                                          xrCore.GetXrViewConfigurationType(), 0, &viewCount, nullptr) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get view configuration views");
    }

    xrCore.GetXRViewConfigurationView().resize(viewCount);
    for (XrViewConfigurationView& viewInfo : xrCore.GetXRViewConfigurationView()) {
        viewInfo.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
        viewInfo.next = nullptr;
    }

    xrCore.GetXrViews().resize(viewCount);
    for (XrView& view : xrCore.GetXrViews()) {
        view.type = XR_TYPE_VIEW;
        view.next = nullptr;
    }

    if (xrEnumerateViewConfigurationViews(xrCore.GetXRInstance(), xrCore.GetSystemID(),
                                          xrCore.GetXrViewConfigurationType(), viewCount, &viewCount,
                                          xrCore.GetXRViewConfigurationView().data()) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get view configuration views");
    }

    XrResult result;
    XrSwapchainCreateInfo swapchainCreateInfo{};
    swapchainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
    swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
    swapchainCreateInfo.format = xrCore.SwapchainFormats().at(0);
    swapchainCreateInfo.sampleCount = xrCore.GetXRViewConfigurationView()[0].recommendedSwapchainSampleCount;
    swapchainCreateInfo.width = xrCore.GetXRViewConfigurationView()[0].recommendedImageRectWidth;
    swapchainCreateInfo.height = xrCore.GetXRViewConfigurationView()[0].recommendedImageRectHeight;
    swapchainCreateInfo.faceCount = 1;
    swapchainCreateInfo.arraySize = viewCount;
    swapchainCreateInfo.mipCount = 1;
    if ((result = xrCreateSwapchain(xrCore.GetXRSession(), &swapchainCreateInfo, &xrCore.GetXrSwapchain())) !=
        XR_SUCCESS) {
        Util::ErrorPopup("Failed to create swapchain");
    }
}

void XrBackend::PrepareXrSwapchainImages() {
    XrResult result;

    uint32_t swapchainImageCount;
    if ((result = xrEnumerateSwapchainImages(xrCore.GetXrSwapchain(), 0, &swapchainImageCount, nullptr)) !=
        XR_SUCCESS) {
        Util::ErrorPopup("Failed to enumerate swapchain images");
    }

    xrCore.GetSwapchainImages().resize(swapchainImageCount);
    for (XrSwapchainImageVulkanKHR& swapchainImage : xrCore.GetSwapchainImages()) {
        swapchainImage.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
    }

    XrSwapchainImageBaseHeader* data =
        reinterpret_cast<XrSwapchainImageBaseHeader*>(xrCore.GetSwapchainImages().data());
    if ((result = xrEnumerateSwapchainImages(xrCore.GetXrSwapchain(), xrCore.GetSwapchainImages().size(),
                                             &swapchainImageCount, data)) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to get swapchain images");
    }

    xrCore.GetCompositionLayerProjectionViews().resize(xrCore.GetXRViewConfigurationView().size());
    for (size_t i = 0; i < xrCore.GetCompositionLayerProjectionViews().size(); ++i) {
        XrCompositionLayerProjectionView& projectionView = xrCore.GetCompositionLayerProjectionViews()[i];
        projectionView.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
        projectionView.next = nullptr;

        projectionView.subImage.swapchain = xrCore.GetXrSwapchain();
        projectionView.subImage.imageArrayIndex = i;
        projectionView.subImage.imageRect.offset = {0, 0};
        projectionView.subImage.imageRect.extent = {
            static_cast<int32_t>(xrCore.GetXRViewConfigurationView()[i].recommendedImageRectWidth),
            static_cast<int32_t>(xrCore.GetXRViewConfigurationView()[i].recommendedImageRectHeight),
        };
    }
}

XrResult XrBackend::StartFrame(uint32_t& imageIndex) {
    frameStarted = false;
    PollEvents();
    if (!this->sessionRunning)
        return XR_ERROR_RUNTIME_FAILURE;

    XrResult result;
    auto sessionState = xrCore.GetXrSessionState();
    if (sessionState != XR_SESSION_STATE_READY && sessionState != XR_SESSION_STATE_SYNCHRONIZED &&
        sessionState != XR_SESSION_STATE_VISIBLE && sessionState != XR_SESSION_STATE_FOCUSED) {
        return XR_ERROR_SESSION_NOT_READY;
    }

    XrFrameWaitInfo frameWaitInfo{XR_TYPE_FRAME_WAIT_INFO};
    xrCore.GetXrFrameState().type = XR_TYPE_FRAME_STATE;
    if ((result = xrWaitFrame(xrCore.GetXRSession(), &frameWaitInfo, &xrCore.GetXrFrameState())) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to wait frame";
        return result;
    }

    XrFrameBeginInfo frameBeginInfo{XR_TYPE_FRAME_BEGIN_INFO};
    if ((result = xrBeginFrame(xrCore.GetXRSession(), &frameBeginInfo)) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to begin frame";
        return XR_ERROR_RUNTIME_UNAVAILABLE;
    }
    frameStarted = true;

    if (!xrCore.GetXrFrameState().shouldRender)
        return XR_ERROR_RUNTIME_FAILURE;

    UpdateViews();

    XrSwapchainImageAcquireInfo swapchainImageAcquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
    if ((result = xrAcquireSwapchainImage(xrCore.GetXrSwapchain(), &swapchainImageAcquireInfo, &imageIndex)) !=
        XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to get acquire swapchain";
        return result;
    }
    XrSwapchainImageWaitInfo swapchainImageWaitInfo{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
    swapchainImageWaitInfo.timeout = XR_INFINITE_DURATION;
    if ((result = xrWaitSwapchainImage(xrCore.GetXrSwapchain(), &swapchainImageWaitInfo)) != XR_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to wait swapchain image";
        return result;
    }

    input.UpdateInput();

    return XR_SUCCESS;
}

XrResult XrBackend::EndFrame(uint32_t& imageIndex) {
    if (!frameStarted)
        return XR_SUCCESS;
    XrResult result;
    XrSwapchainImageReleaseInfo swapchainImageReleaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
    if ((result = xrReleaseSwapchainImage(xrCore.GetXrSwapchain(), &swapchainImageReleaseInfo)) != XR_SUCCESS) {
        LOGGER(LOGGER::WARNING) << "Failed to release swapchain image" << result;
    }
    XrCompositionLayerProjection compositionLayerProjection{XR_TYPE_COMPOSITION_LAYER_PROJECTION};
    compositionLayerProjection.space = xrCore.GetXrSpace();
    compositionLayerProjection.viewCount = xrCore.GetXRViewConfigurationView().size();
    compositionLayerProjection.views = xrCore.GetCompositionLayerProjectionViews().data();

    std::vector<XrCompositionLayerBaseHeader*> layers;
    if (xrCore.GetXrFrameState().shouldRender) {
        layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&compositionLayerProjection));
    }

    XrFrameEndInfo frameEndInfo{XR_TYPE_FRAME_END_INFO};
    frameEndInfo.displayTime = xrCore.GetXrFrameState().predictedDisplayTime;
    frameEndInfo.layerCount = layers.size();
    frameEndInfo.layers = layers.data();
    frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    if ((result = xrEndFrame(xrCore.GetXRSession(), &frameEndInfo)) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to end frame");
    }

    return XR_SUCCESS;
}

void XrBackend::BeginSession() {
    XrResult result;
    XrSessionBeginInfo sessionBeginInfo{};
    sessionBeginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
    sessionBeginInfo.primaryViewConfigurationType = xrCore.GetXrViewConfigurationType();

    if ((result = xrBeginSession(xrCore.GetXRSession(), &sessionBeginInfo))) {
        Util::ErrorPopup("Failed to begin xr session");
    }

    sessionRunning = true;
}

void XrBackend::EndSession() {
    XrResult result;
    if ((result = xrEndSession(xrCore.GetXRSession())) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to end xr session");
    }

    sessionRunning = false;
}

void XrBackend::PollEvents() {
    XrEventDataBuffer eventData{XR_TYPE_EVENT_DATA_BUFFER};
    auto XrPoolEvent = [&]() -> bool {
        eventData = {XR_TYPE_EVENT_DATA_BUFFER};
        return xrPollEvent(xrCore.GetXRInstance(), &eventData) == XR_SUCCESS;
    };

    while (XrPoolEvent()) {
        switch (eventData.type) {
            case XR_TYPE_EVENT_DATA_EVENTS_LOST: {
                XrEventDataEventsLost* eventsLost = reinterpret_cast<XrEventDataEventsLost*>(&eventData);
                LOGGER(LOGGER::INFO) << "OpenXR events lost: " << eventsLost->lostEventCount;
                xrShouldStop = true;
                break;
            }
            case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
                XrEventDataSessionStateChanged* sessionStateChanged =
                    reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
                if (sessionStateChanged->session != xrCore.GetXRSession()) {
                    LOGGER(LOGGER::WARNING) << "XrEventDatasessionStateChanged for unknow session";
                    break;
                }
                if (sessionStateChanged->state == XR_SESSION_STATE_READY) {
                    BeginSession();
                }
                if (sessionStateChanged->state == XR_SESSION_STATE_STOPPING) {
                    EndSession();
                }
                if (sessionStateChanged->state == XR_SESSION_STATE_EXITING ||
                    sessionStateChanged->state == XR_SESSION_STATE_LOSS_PENDING) {
                    xrShouldStop = true;
                }
                xrCore.GetXrSessionState() = sessionStateChanged->state;
                break;
            }
            case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
                break;
            default:
                LOGGER(LOGGER::INFO) << "Unhandled event data received" << eventData.type;
                break;
        }
    }
}

void XrBackend::UpdateViews() {
    XrResult result;
    xrCore.GetXrViewState().type = XR_TYPE_VIEW_STATE;
    XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO};
    viewLocateInfo.viewConfigurationType = xrCore.GetXrViewConfigurationType();
    viewLocateInfo.displayTime = xrCore.GetXrFrameState().predictedDisplayTime;
    viewLocateInfo.space = xrCore.GetXrSpace();
    if ((result = xrLocateViews(xrCore.GetXRSession(), &viewLocateInfo, &xrCore.GetXrViewState(),
                                xrCore.GetXrViews().size(), &viewCount, xrCore.GetXrViews().data())) != XR_SUCCESS) {
        Util::ErrorPopup("Failed to locate views");
    }

    if (viewCount != xrCore.GetXRViewConfigurationView().size()) {
        Util::ErrorPopup("View size changed!");
    }

    auto viewState = xrCore.GetXrViewState();
    if ((viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) == 0 ||
        (viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT) == 0) {
        Util::ErrorPopup("Invalid view state");
    }

    std::vector<glm::mat4> viewMatrices(2);
    std::vector<glm::mat4> projectionMatrices(2);

    for (size_t i = 0; i < viewCount; ++i) {
        XrCompositionLayerProjectionView& cpLayerProjectionView = xrCore.GetCompositionLayerProjectionViews()[i];
        XrView& view = xrCore.GetXrViews()[i];
        cpLayerProjectionView.pose = view.pose;
        cpLayerProjectionView.fov = view.fov;

        viewMatrices[i] = glm::inverse(MathUtil::XrPoseToMatrix(cpLayerProjectionView.pose));
        projectionMatrices[i] = MathUtil::XrCreateProjectionMatrix(cpLayerProjectionView.fov, 0.01f, 1000.0f);
    }

    EventSystem::TriggerEvent(Events::XRLIB_EVENT_HEAD_MOVEMENT, viewMatrices, projectionMatrices);
}
}    // namespace XR
}    // namespace XRLib
