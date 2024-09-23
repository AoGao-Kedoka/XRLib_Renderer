#pragma once

/// <summary>
/// Events can be used by the user
/// </summary>
namespace XRLib {
class Events {
   public:
    // Application events
    inline static std::string XRLIB_EVENT_APPLICATION_START{"application_start"};
    inline static std::string XRLIB_EVENT_APPLICATION_PREPARE_FINISHED{"application_prepare_finished"};
    inline static std::string XRLIB_EVENT_APPLICATION_PRE_RENDERING{"application_pre_rendering"};
    inline static std::string XRLIB_EVENT_APPLICATION_POST_RENDERING{"application_post_rendering"};

    // backends events
    inline static std::string XRLIB_EVENT_RENDERBACKEND_INIT_FINISHED{"renderbackend_init_finished"};
    inline static std::string XRLIB_EVENT_XRBACKEND_INIT_FINISHED{"xrbackend_init_finished"};

    // Window events
    inline static std::string XRLIB_EVENT_WINDOW_RESIZED{"window_resized"};

    // input
    inline static std::string XRLIB_EVENT_KEY_PRESSED{"window_key_pressed"};

    inline static std::string XRLIB_EVENT_MOUSE_LEFT_DOWN_EVENT{"window_mouse_left_down"};
    inline static std::string XRLIB_EVENT_MOUSE_LEFT_MOVEMENT_EVENT{"window_mouse_left_movement"};
    inline static std::string XRLIB_EVENT_MOUSE_RIGHT_DOWN_EVENT{"window_mouse_right_down"};
    inline static std::string XRLIB_EVENT_MOUSE_RIGHT_MOVEMENT_EVENT{"window_mouse_right_movement"};

    inline static std::string XRLIB_EVENT_HEAD_MOVEMENT{"head_movement"};

    // Controller events
    inline static std::string XRLIB_EVENT_LEFT_CONTROLLER_POSITION{"left_controller_position"};
    inline static std::string XRLIB_EVENT_RIGHT_CONTROLLER_POSITION{"right_controller_position"};

    inline static std::string XRLIB_EVENT_LEFT_TRIGGER_PRESSED{"left_trigger_pressed"};
    inline static std::string XRLIB_EVENT_RIGHT_TRIGGER_PRESSED{"right_trigger_pressed"};
    inline static std::string XRLIB_EVENT_LEFT_GRIP_PRESSED{"left_grip_pressed"};
    inline static std::string XRLIB_EVENT_RIGHT_GRIP_PRESSED{"right_grip_pressed"};
};
}    // namespace XRLib