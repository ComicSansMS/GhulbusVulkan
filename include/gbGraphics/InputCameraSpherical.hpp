#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_INPUT_CAMERA_SPHERICAL_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_INPUT_CAMERA_SPHERICAL_HPP

/** @file
*
* @brief Input Camera Spherical.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/WindowEventReactor.hpp>

#include <gbMath/Transform3.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class Window;

namespace Input {
class CameraSpherical {
public:
    struct Config {
        bool useMouseMove = true;
        bool useMouseZoom = true;
        bool useMouseWheelZoom = true;
        float speedHorizontal = 1.f;
        float speedVertical = 1.f;
        float speedZoom = 1.f;
        float speedMouseWheelZoom = 1.f;
        MouseButton mouseRotateButton = MouseButton::Left;
        MouseButton mouseZoomButton = MouseButton::Right;
        float cameraDistanceMin = 1.f;
        float cameraDistanceMax = 100.f;
    };
private:
    Window* m_window;
    WindowEventReactor::MouseMoveGuard m_mouseMoveGuard;
    WindowEventReactor::MouseClickGuard m_mouseClickGuard;
    WindowEventReactor::MouseScrollGuard m_mouseScrollGuard;
    Config m_config;
    GhulbusMath::Transform3<float> m_transform;
    float m_cameraAngleHorizontal;
    float m_cameraAngleVertical;
    float m_cameraDistance;
    bool m_transformIsValid;
    bool m_inRotate;
    bool m_inZoom;
    GhulbusMath::Vector2d m_lastPosition;
public:
    CameraSpherical(Window& window);

    void onMouseMove(Event::MouseMove const& mouse_move_event);
    void onMouseClick(Event::MouseClick const& mouse_click_event);
    void onMouseScroll(Event::MouseScroll const& mouse_scroll_event);

    GhulbusMath::Transform3<float> getTransform();
private:
    void updateCamera();
};
}
}
#endif
