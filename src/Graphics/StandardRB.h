#pragma once

// Standard rendering behaviour, should be ignored if custom render pass is defined
namespace XRLib {
namespace Graphics {
class StandardRB {
   public:
    virtual ~StandardRB() = default;

    virtual bool StartFrame(uint32_t& imageIndex) = 0;
    virtual void RecordFrame(uint32_t& imageIndex) = 0;
    virtual void EndFrame(uint32_t& imageIndex) = 0;
};
}    // namespace Graphics
}    // namespace XRLib