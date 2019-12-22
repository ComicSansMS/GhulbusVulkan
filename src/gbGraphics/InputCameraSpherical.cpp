#include <gbGraphics/InputCameraSpherical.hpp>

#include <gbGraphics/Window.hpp>

#include <gbMath/NumberTypeTraits.hpp>
#include <gbMath/Vector3.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace GHULBUS_GRAPHICS_NAMESPACE::Input
{
CameraSpherical::CameraSpherical(Window& window)
    :m_window(&window),
    m_mouseMoveGuard(window.getEventReactor().eventHandlers.mouseMoveEvent.addHandler([this](auto e) {
        onMouseMove(e);
        return WindowEventReactor::Result::ContinueProcessing;
    })),
    m_mouseClickGuard(window.getEventReactor().eventHandlers.mouseClickEvent.addHandler([this](auto e) {
        onMouseClick(e);
        return WindowEventReactor::Result::ContinueProcessing;
    })),
    m_mouseScrollGuard(window.getEventReactor().eventHandlers.mouseScrollEvent.addHandler([this](auto e) {
        onMouseScroll(e);
        return WindowEventReactor::Result::ContinueProcessing;
    })), m_cameraAngleHorizontal(0.f), m_cameraAngleVertical(0.f), m_cameraDistance(5.f), m_transformIsValid(false),
    m_inRotate(false), m_inZoom(false)
{
}

void CameraSpherical::setCameraAngleHorizontal(float phi)
{
    float constexpr pi_2 = GhulbusMath::traits::Pi<float>::value * 2.f;
    m_cameraAngleHorizontal = std::fmod(phi, pi_2);
    m_transformIsValid = false;
}

void CameraSpherical::setCameraAngleVertical(float theta)
{
    float constexpr pi_1_2 = GhulbusMath::traits::Pi<float>::value / 2.f;
    float constexpr epsilon = std::numeric_limits<float>::epsilon() * pi_1_2;
    m_cameraAngleVertical = std::clamp(theta, -pi_1_2 + epsilon, pi_1_2 - epsilon);
    m_transformIsValid = false;
}

void CameraSpherical::setCameraDistance(float d)
{
    m_cameraDistance = std::clamp(d, config.cameraDistanceMin, config.cameraDistanceMax);
    m_transformIsValid = false;
}

void CameraSpherical::onMouseMove(Event::MouseMove const& mouse_move_event)
{
    GhulbusMath::Vector2d const delta = mouse_move_event.position - m_lastPosition;
    if (config.useMouseMove && m_inRotate) {
        m_cameraAngleHorizontal += static_cast<float>(delta.x) * 0.01f * config.speedHorizontal;
        m_cameraAngleVertical += static_cast<float>(delta.y) * 0.01f * config.speedVertical;
        m_transformIsValid = false;
    } else if (config.useMouseZoom && m_inZoom) {
        m_cameraDistance += static_cast<float>(delta.y) * 0.005f * m_cameraDistance * config.speedZoom;
        m_transformIsValid = false;
    }
    m_lastPosition = mouse_move_event.position;
}

void CameraSpherical::onMouseClick(Event::MouseClick const& mouse_click_event)
{
    if (mouse_click_event.button == config.mouseZoomButton) {
        m_inZoom = (mouse_click_event.action == MouseButtonAction::Press);
    } else if (mouse_click_event.button == config.mouseRotateButton) {
        m_inRotate = (mouse_click_event.action == MouseButtonAction::Press);
    }
    if(m_inZoom || m_inRotate) {
        if(!m_window->isCursorDisabled()) {
            m_window->disableCursor(true);
            m_window->setMouseMotionRaw(true);
        }
    } else {
        if(m_window->isCursorDisabled()) {
            m_window->disableCursor(false);
            m_window->setMouseMotionRaw(false);
        }
    }
}

void CameraSpherical::onMouseScroll(Event::MouseScroll const& mouse_scroll_event)
{
    if (config.useMouseWheelZoom) {
        float const scroll_offset = static_cast<float>(mouse_scroll_event.offset.y);
        m_cameraDistance += scroll_offset * -0.1f * m_cameraDistance * config.speedMouseWheelZoom;
        m_transformIsValid = false;
    }
}

GhulbusMath::Transform3<float> CameraSpherical::getTransform()
{
    if (!m_transformIsValid) { updateCamera(); }
    return m_transform;
}

void CameraSpherical::updateCamera()
{
    m_cameraDistance = std::clamp(m_cameraDistance, config.cameraDistanceMin, config.cameraDistanceMax);
    float constexpr pi_2 = GhulbusMath::traits::Pi<float>::value * 2.f;
    float constexpr pi_1_2 = GhulbusMath::traits::Pi<float>::value / 2.f;
    float constexpr epsilon = std::numeric_limits<float>::epsilon() * pi_1_2;
    if (m_cameraAngleHorizontal <= -pi_2) { m_cameraAngleHorizontal += pi_2; }
    if (m_cameraAngleHorizontal >= pi_2) { m_cameraAngleHorizontal -= pi_2; }
    m_cameraAngleVertical = std::clamp(m_cameraAngleVertical, -pi_1_2 + epsilon, pi_1_2 - epsilon);
    GhulbusMath::Vector3f const camera_position(
        -m_cameraDistance * std::cos(m_cameraAngleHorizontal) * std::sin(m_cameraAngleVertical - pi_1_2),
         m_cameraDistance * std::cos(m_cameraAngleVertical - pi_1_2),
         m_cameraDistance * std::sin(m_cameraAngleHorizontal) * std::sin(m_cameraAngleVertical - pi_1_2));

    m_transform = GhulbusMath::make_view_look_at(camera_position, 
                                                 GhulbusMath::Vector3f(0.f, 0.f, 0.f),
                                                 GhulbusMath::Vector3f(0.f, 1.f, 0.f));
    m_transformIsValid = true;
}
}
