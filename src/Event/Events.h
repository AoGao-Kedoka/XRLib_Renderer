#pragma once

namespace XRLib {
class Events {

   public:
    // Renderbackend events
    inline static std::string XRLIB_EVENT_RENDERBACKEND_INIT_FINISHED{
        "renderbackend_init_finished"};
    // Window events
    inline static std::string XRLIB_EVENT_WINDOW_RESIZED{"window_resized"};

    // input
    inline static std::string XRLIB_EVENT_KEY_PRESSED{"window_key_pressed"};

    inline static std::string XRLIB_EVENT_MOUSE_LEFT_DOWN_EVENT{"window_mouse_left_down"};
    inline static std::string XRLIB_EVENT_MOUSE_LEFT_MOVEMENT_EVENT{"window_mouse_left_movement"};
    inline static std::string XRLIB_EVENT_MOUSE_RIGHT_DOWN_EVENT{"window_mouse_right_down"};
    inline static std::string XRLIB_EVENT_MOUSE_RIGHT_MOVEMENT_EVENT{"window_mouse_right_movement"};

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