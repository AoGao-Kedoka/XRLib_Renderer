#include "RenderBackend.h"
#include "Utils/Util.h"

namespace XRLib {
namespace Graphics {
RenderBackend::RenderBackend(Config& info, VkCore& vkCore, XR::XrCore& xrCore, XRLib::Scene& scene)
    : info{info}, vkCore{vkCore}, xrCore{xrCore}, scene{scene} {

    InitVulkan();

    EventSystem::TriggerEvent(Events::XRLIB_EVENT_RENDERBACKEND_INIT_FINISHED);
}

RenderBackend::~RenderBackend() {}

void RenderBackend::InitVulkan() {
    vkCore.CreateVkInstance(info, xrCore.VkAdditionalInstanceExts2());
    if (xrCore.IsXRValid())
        xrCore.VkSetPhysicalDevice(vkCore.GetRenderInstance(), &vkCore.VkPhysicalDeviceRef());
    else
        vkCore.SelectPhysicalDevice();
    vkCore.CreateVkDevice(info, xrCore.VkAdditionalDeviceExts2(), xrCore.IsXRValid());
}

void RenderBackend::Prepare() {
    if (xrCore.IsXRValid()) {
        GetSwapchainConfig();
    }

    // vulkan prepare
    VkStandardRB* vkSRB = dynamic_cast<VkStandardRB*>(renderBehavior.get());
    vkSRB->InitVerticesIndicesBuffers();
    vkSRB->Prepare();
}

void RenderBackend::GetSwapchainConfig() {
    std::vector<std::unique_ptr<Image>> swapchainImages;
    auto [width, height] = xrCore.SwapchainExtent();
    for (int i = 0; i < xrCore.GetSwapchainImages().size(); ++i) {
        swapchainImages.push_back(std::make_unique<Image>(vkCore, xrCore.GetSwapchainImages()[i].image,
                                                          static_cast<VkFormat>(xrCore.SwapchainFormats()[0]), width,
                                                          height, 2));
    }
    VkStandardRB* vkSRB = dynamic_cast<VkStandardRB*>(renderBehavior.get());
    vkSRB->GetSwapchain() = std::make_unique<Swapchain>(vkCore, swapchainImages);
}

bool RenderBackend::StartFrame(uint32_t& imageIndex) {
    return renderBehavior->StartFrame(imageIndex);
}

void RenderBackend::RecordFrame(uint32_t& imageIndex) {
    renderBehavior->RecordFrame(imageIndex);
}

void RenderBackend::EndFrame(uint32_t& imageIndex) {
    renderBehavior->EndFrame(imageIndex);
}
}    // namespace Graphics
}    // namespace XRLib
