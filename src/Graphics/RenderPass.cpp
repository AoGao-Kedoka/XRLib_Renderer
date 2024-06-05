#include "RenderPass.h"

RenderPass::RenderPass(std::shared_ptr<VkCore> core) : core{core} {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = core->GetFlatSwapchainImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(core->GetRenderDevice(), &renderPassInfo, nullptr,
                           &pass) != VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create render pass";
        exit(-1);
    }
}

RenderPass::~RenderPass() {
    LOGGER(LOGGER::DEBUG) << "render pass destructor called";
    if (!core)
        return;
    Util::VkSafeClean(vkDestroyRenderPass, core->GetRenderDevice(), pass,
                      nullptr);
}

void RenderPass::Record(uint32_t imageIndex) {

    if (graphicsPipeline == VK_NULL_HANDLE) {
        LOGGER(LOGGER::ERR) << "Graphics pipeline not initialized";
        exit(-1);
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                     // Optional
    beginInfo.pInheritanceInfo = nullptr;    // Optional

    if (vkBeginCommandBuffer(core->GetCommandBuffer(), &beginInfo) !=
        VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to begin recording command buffer";
    }
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pass;
    renderPassInfo.framebuffer =
        core->GetSwapchainFrameBufferFlat()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = core->GetFlatSwapchainExtent2D();

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(core->GetCommandBuffer(), &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(core->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                      *graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(core->GetFlatSwapchainExtent2D().width);
    viewport.height =
        static_cast<float>(core->GetFlatSwapchainExtent2D().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(core->GetCommandBuffer(), 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = core->GetFlatSwapchainExtent2D();
    vkCmdSetScissor(core->GetCommandBuffer(), 0, 1, &scissor);

    //TODO Bind vertex buffer,index buffer and descriptor sets

    vkCmdDraw(core->GetCommandBuffer(), 3, 1, 0, 0);

    vkCmdEndRenderPass(core->GetCommandBuffer());

    if (vkEndCommandBuffer(core->GetCommandBuffer()) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}
