#pragma once

namespace XRLib {
class XRLib;

class Time {
   public:
    static float GetDeltaTime() { return deltaTime; }

   private:
    static float deltaTime;

    friend class XRLib;

    static void UpdateDeltaTime(float newDeltaTime) { deltaTime = newDeltaTime; }
};
}    // namespace XRLib