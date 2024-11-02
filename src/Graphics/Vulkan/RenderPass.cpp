#include "RenderPass.h"

namespace XRLib {
namespace Graphics {
RenderPass::RenderPass(std::shared_ptr<VkCore> core, std::vector<std::unique_ptr<Image>>& renderTargets, bool multiview)
    : core{core}, multiview{multiview}, renderTargets{renderTargets} {
    depthImage = std::make_unique<Image>(core, renderTargets[0]->Width(), renderTargets[0]->Height(),
                                         VkUtil::FindDepthFormat(core->GetRenderPhysicalDevice()),
                                         VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, multiview ? 2 : 1);
    CreateRenderPass();
    SetRenderTarget(renderTargets);

    EventSystem::Callback<int, int> windowResizeCallback = [this, core, multiview](int width, int height) {
        vkDeviceWaitIdle(core->GetRenderDevice());
        CleanupFrameBuffers();
        depthImage = std::make_unique<Image>(
            core, width, height, VkUtil::FindDepthFormat(core->GetRenderPhysicalDevice()), VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, multiview ? 2 : 1);

        // when render target is not swapchain
        for (const auto& renderTarget : this->GetRenderTargets()) {
            if (renderTarget->Width() != width || renderTarget->Height() != height) {
                //TODO resize image
            }
        }

        SetRenderTarget(this->GetRenderTargets());
    };
    EventSystem::RegisterListener<int, int>(Events::XRLIB_EVENT_WINDOW_RESIZED, windowResizeCallback);
}

RenderPass::~RenderPass() {
    if (!core)
        return;
    CleanupFrameBuffers();
    VkUtil::VkSafeClean(vkDestroyRenderPass, core->GetRenderDevice(), pass, nullptr);
}
void RenderPass::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = renderTargets[0]->GetFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout =
        multiview ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = depthImage->GetFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    if (multiview) {
        constexpr uint32_t viewMask = 0b00000011;
        constexpr uint32_t correlationMask = 0b00000011;

        VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo{
            VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO};
        renderPassMultiviewCreateInfo.subpassCount = 1;
        renderPassMultiviewCreateInfo.pViewMasks = &viewMask;
        renderPassMultiviewCreateInfo.correlationMaskCount = 1;
        renderPassMultiviewCreateInfo.pCorrelationMasks = &correlationMask;

        renderPassInfo.pNext = &renderPassMultiviewCreateInfo;
    }

    if (vkCreateRenderPass(core->GetRenderDevice(), &renderPassInfo, nullptr, &pass) != VK_SUCCESS) {
        Util::ErrorPopup("Failed to create render pass");
    }
}

void RenderPass::SetRenderTarget(std::vector<std::unique_ptr<Image>>& images) {
    frameBuffers.resize(images.size());
    for (int i = 0; i < images.size(); ++i) {
        std::vector<VkImageView> attachments = {images[i]->GetImageView(),
                                                depthImage->GetImageView(VK_IMAGE_ASPECT_DEPTH_BIT)};

        VkFramebufferCreateInfo framebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferCreateInfo.renderPass = GetVkRenderPass();
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferCreateInfo.pAttachments = attachments.data();
        framebufferCreateInfo.width = images[i]->Width();
        framebufferCreateInfo.height = images[i]->Height();
        framebufferCreateInfo.layers = 1;

        VkResult result =
            vkCreateFramebuffer(core->GetRenderDevice(), &framebufferCreateInfo, nullptr, &frameBuffers[i]);
        if (result != VK_SUCCESS) {
            Util::ErrorPopup("Failed to create frame buffer");
        }
    }
}

void RenderPass::CleanupFrameBuffers() {
    for (const auto& frameBuffer : frameBuffers) {
        VkUtil::VkSafeClean(vkDestroyFramebuffer, core->GetRenderDevice(), frameBuffer, nullptr);
    }
    frameBuffers.clear();
}
std::vector<std::unique_ptr<Image>>& RenderPass::GetRenderTargets() {
    return renderTargets;
}

}    // namespace Graphics
}    // namespace XRLib
