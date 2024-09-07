#pragma once

#include "Event/EventSystem.h"
#include "Utils/Transform.h"
#include "XrCore.h"

namespace XRLib {
namespace XR {
class XrInput {
   public:
    XrInput(std::shared_ptr<XrCore> core);
    ~XrInput() = default;
    void FetchInput();

    Transform GetLeftControllerPosition() { return leftControllerPos; }
    Transform GetRightControllerPosition() { return rightControllerPos; }

   private:
    std::shared_ptr<XrCore> core;

    Transform leftControllerPos;
    Transform rightControllerPos;
    Transform headPos;
};
}    // namespace XR
}    // namespace XRLib
