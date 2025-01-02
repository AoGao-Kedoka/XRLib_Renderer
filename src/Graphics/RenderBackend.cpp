#include "RenderBackend.h"
#include "Utils/Util.h"

namespace XRLib {
namespace Graphics {
RenderBackend::RenderBackend(Info& info, VkCore& vkCore, XR::XrCore& xrCore, XRLib::Scene& scene)
    : info{info}, vkCore{vkCore}, xrCore{xrCore}, scene{scene} {

    InitVulkan();

    EventSystem::TriggerEvent(Events::XRLIB_EVENT_RENDERBACKEND_INIT_FINISHED);

    if (xrCore.IsXRValid()) {
        GetSwapchainInfo();
    }
}

RenderBackend::~RenderBackend() {}

void RenderBackend::InitVulkan() {
    vkCore.CreateVkInstance(info, xrCore.VkAdditionalInstanceExts());
    if (xrCore.IsXRValid())
        xrCore.VkSetPhysicalDevice(vkCore.GetRenderInstance(), &vkCore.VkPhysicalDeviceRef());
    else
        vkCore.SelectPhysicalDevice();
    vkCore.CreateVkDevice(info, xrCore.VkAdditionalDeviceExts(), xrCore.IsXRValid());
}

void RenderBackend::Prepare()
{
    vkSRB.InitVerticesIndicesShader();
    vkSRB.PrepareDefaultStereoRenderPasses(viewProj, RenderPasses);
}

void RenderBackend::Prepare(std::vector<std::unique_ptr<IGraphicsRenderpass>>& passes) {
    LOGGER(LOGGER::INFO) << "Using custom render pass";
    this->RenderPasses = std::move(passes);
}

void RenderBackend::GetSwapchainInfo() {
    std::vector<std::unique_ptr<Image>> swapchainImages;
    auto [width, height] = xrCore.SwapchainExtent();
    for (int i = 0; i < xrCore.GetSwapchainImages().size(); ++i) {
        swapchainImages.push_back(std::make_unique<Image>(vkCore, xrCore.GetSwapchainImages()[i].image,
                                                          static_cast<VkFormat>(xrCore.SwapchainFormats()[0]), width,
                                                          height, 2));
    }
    vkSRB.GetSwapchain() = std::make_unique<Swapchain>(vkCore, swapchainImages);
}

bool RenderBackend::StartFrame(uint32_t& imageIndex) {
    return vkSRB.StartFrame(imageIndex);
}

void RenderBackend::RecordFrame(uint32_t& imageIndex) {
    vkSRB.RecordFrame(imageIndex);
}

void RenderBackend::RecordFrame(uint32_t& imageIndex,
                                std::function<void(uint32_t&, CommandBuffer&)> recordingFunction) {
    vkResetFences(vkCore.GetRenderDevice(), 1, &vkCore.GetInFlightFence());
    CommandBuffer commandBuffer{vkCore};
    vkResetCommandBuffer(commandBuffer.GetCommandBuffer(), 0);

    // custom frame recording
    recordingFunction(imageIndex, commandBuffer);
}

void RenderBackend::EndFrame(uint32_t& imageIndex) {
    vkSRB.EndFrame(imageIndex);
}
}    // namespace Graphics
}    // namespace XRLib
