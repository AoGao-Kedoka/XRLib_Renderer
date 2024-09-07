#pragma once

namespace XRLib {
class Events {

   public:
    // Renderbackend events
    inline static std::string XRLIB_EVENT_RENDERBACKEND_INIT_FINISHED{
        "renderbackend_init_finished"};
    // Window events
    inline static std::string XRLIB_EVENT_WINDOW_RESIZED{"window_resized"};

    // Controller events
    inline static std::string XRLIB_EVENT_LEFT_TRIGGER_PRESSED{
        "left_trigger_pressed"};
    inline static std::string XRLIB_EVENT_RIGHT_TRIGGER_PRESSED{
        "right_trigger_pressed"};
    inline static std::string XRLIB_EVENT_LEFT_GRIP_PRESSED{
        "left_grip_pressed"};
    inline static std::string XRLIB_EVENT_RIGHT_GRIP_PRESSED{
        "right_grip_pressed"};
};
}    // namespace XRLib