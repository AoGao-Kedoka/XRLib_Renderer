#pragma once

#include "Utils/Event.h"
#include "Utils/Transform.h"
#include "XrCore.h"

#define XRLIB_EVENT_LEFT_TRIGGER_PRESSED "left_trigger_pressed"
#define XRLIB_EVENT_RIGHT_TRIGGER_PRESSED "right_trigger_pressed"
#define XRLIB_EVENT_LEFT_GRIP_PRESSED "left_grip_pressed"
#define XRLIB_EVENT_RIGHT_GRIP_PRESSED "right_grip_pressed"

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