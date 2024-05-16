#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <utility>
#include <vector>

#include "Core.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Shader.h"
#include "Info.h"
#include "Logger.h"

class RenderBackend {
   public:
    RenderBackend(Info& info, Core& core);
    ~RenderBackend();

    RenderBackend(RenderBackend&& src) noexcept
        : info(std::exchange(src.info, nullptr)),
          core(std::exchange(src.core, nullptr)),
          window(std::exchange(src.window, nullptr)),
          vkDebugMessenger(std::exchange(src.vkDebugMessenger, VK_NULL_HANDLE)),
          vkCreateDebugUtilsMessengerEXT(
              std::exchange(src.vkCreateDebugUtilsMessengerEXT, nullptr)) {
        LOGGER(LOGGER::DEBUG) << "Move constructor called";
    }

    RenderBackend& operator=(RenderBackend&& rhs) noexcept {
        if (this == &rhs)
            return *this;

        LOGGER(LOGGER::DEBUG) << "Move assignment called";
        info = std::exchange(rhs.info, nullptr);
        core = std::exchange(rhs.core, nullptr);
        vkDebugMessenger = std::exchange(rhs.vkDebugMessenger, VK_NULL_HANDLE);
        vkCreateDebugUtilsMessengerEXT =
            std::exchange(rhs.vkCreateDebugUtilsMessengerEXT, nullptr);
        return *this;
    }

    void CreateVulkanInstance();
    void CreatePhysicalDevice();
    void CreateLogicalDevice();

    virtual void Prepare(){};
    virtual void CreateRenderPass(std::string vertexShaderPath,
                                  std::string fragmentShaderPath){};

    struct GraphicsRenderPass {
        GraphicsRenderPass(Core* core, std::string vertexShaderPath,
                           std::string fragmentShaderPath)
            : core{core},
              vertexShader{core, vertexShaderPath, Shader::VERTEX_SHADER},
              fragmentShader{core, fragmentShaderPath,
                             Shader::FRAGMENT_SHADER} {
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
            colorAttachmentRef.layout =
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;

            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = &colorAttachment;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;

            if (vkCreateRenderPass(core->GetRenderDevice(), &renderPassInfo,
                                   nullptr, &renderPass) != VK_SUCCESS) {
                LOGGER(LOGGER::ERR) << "Failed to create render pass";
                exit(-1);
            }

            pipeline = Pipeline{core, vertexShader, fragmentShader, renderPass};
        }

        ~GraphicsRenderPass() {
            Util::VkSafeClean(vkDestroyRenderPass, core->GetRenderDevice(),
                              renderPass, nullptr);
        }

        Core* core;
        VkRenderPass renderPass{VK_NULL_HANDLE};
        Shader vertexShader;
        Shader fragmentShader;
        Pipeline pipeline;
    };
    std::vector<GraphicsRenderPass> renderPasses;

   protected:
    Info* info;
    Core* core;
    GLFWwindow* window;

   private:
    const std::vector<const char*> validataionLayers = {
        "VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT vkDebugMessenger{};

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{nullptr};
};
