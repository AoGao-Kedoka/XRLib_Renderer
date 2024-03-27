#include "RenderBackend.h"

RenderBackend::RenderBackend(Info& info) : info{&info} {}

uint32_t RenderBackend::GetQueueFamilyIndex() {
    return 0;
}

