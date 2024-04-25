#pragma once

#include <algorithm>

#include "RenderBackend.h"

class RenderBackendFlat : public RenderBackend {
   public:
    RenderBackendFlat(Info& info, Core& core) : RenderBackend(info, core) {}
    ~RenderBackendFlat();

    RenderBackendFlat(RenderBackendFlat&& other) noexcept
        : RenderBackend(std::move(other)),
          surface(std::exchange(other.surface, VK_NULL_HANDLE)) {
        LOGGER(LOGGER::DEBUG) << "RenderBackendFlat move constructor called";
    }

    RenderBackendFlat& operator=(RenderBackendFlat&& rhs) noexcept {
        if (this != &rhs) {
            LOGGER(LOGGER::DEBUG) << "RenderBackendFlat move assignment called";
            RenderBackend::operator=(std::move(rhs));
            surface = std::exchange(rhs.surface, VK_NULL_HANDLE);
        }
        return *this;
    }

    void Prepare() override;
    void CreateRenderPass(std::string vertexShaderPath,
                          std::string fragmentShaderPath) override;

   private:
    void CreateFlatSwapChain();
    void PrepareFlatWindow();

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkSwapchainKHR swapChain{VK_NULL_HANDLE};
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat{VK_FORMAT_UNDEFINED};
    VkExtent2D swapChainExtent{0, 0};
    std::vector<VkImageView> swapChainImageViews;
};
