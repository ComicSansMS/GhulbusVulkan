#include <gbGraphics/WindowEvents.hpp>

#include <ostream>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

std::ostream& operator<<(std::ostream& os, Key const& k)
{
    os << '<';
    switch (k) {
    default:
    case Key::Unknown:      os << "Unknown";    break;
    case Key::Space:        os << "Space";      break;
    case Key::Apostrophe:   os << "\'";         break;
    case Key::Comma:        os << ",";          break;
    case Key::Minus:        os << "-";          break;
    case Key::Period:       os << ".";          break;
    case Key::Slash:        os << "/";          break;
    case Key::k0:           os << "0";          break;
    case Key::k1:           os << "1";          break;
    case Key::k2:           os << "2";          break;
    case Key::k3:           os << "3";          break;
    case Key::k4:           os << "4";          break;
    case Key::k5:           os << "5";          break;
    case Key::k6:           os << "6";          break;
    case Key::k7:           os << "7";          break;
    case Key::k8:           os << "8";          break;
    case Key::k9:           os << "9";          break;
    case Key::Semicolon:    os << ";";          break;
    case Key::Equal:        os << "=";          break;
    case Key::A:            os << "A";          break;
    case Key::B:            os << "B";          break;
    case Key::C:            os << "C";          break;
    case Key::D:            os << "D";          break;
    case Key::E:            os << "E";          break;
    case Key::F:            os << "F";          break;
    case Key::G:            os << "G";          break;
    case Key::H:            os << "H";          break;
    case Key::I:            os << "I";          break;
    case Key::J:            os << "J";          break;
    case Key::K:            os << "K";          break;
    case Key::L:            os << "L";          break;
    case Key::M:            os << "M";          break;
    case Key::N:            os << "N";          break;
    case Key::O:            os << "O";          break;
    case Key::P:            os << "P";          break;
    case Key::Q:            os << "Q";          break;
    case Key::R:            os << "R";          break;
    case Key::S:            os << "S";          break;
    case Key::T:            os << "T";          break;
    case Key::U:            os << "U";          break;
    case Key::V:            os << "V";          break;
    case Key::W:            os << "W";          break;
    case Key::X:            os << "X";          break;
    case Key::Y:            os << "Y";          break;
    case Key::Z:            os << "Z";          break;
    case Key::LeftBracket:  os << "[";          break;
    case Key::Backslash:    os << "\\";         break;
    case Key::RightBracket: os << "]";          break;
    case Key::GraveAccent:  os << "`";          break;
    case Key::World1:       os << "World1";     break;
    case Key::World2:       os << "World2";     break;
    case Key::Escape:       os << "Escape";     break;
    case Key::Enter:        os << "Enter";      break;
    case Key::Tab:          os << "Tab";        break;
    case Key::Backspace:    os << "Backspace";  break;
    case Key::Insert:       os << "Ins";        break;
    case Key::Delete:       os << "Del";        break;
    case Key::Right:        os << "Right";      break;
    case Key::Left:         os << "Left";       break;
    case Key::Down:         os << "Down";       break;
    case Key::Up:           os << "Up";         break;
    case Key::PageUp:       os << "PgUp";       break;
    case Key::PageDown:     os << "PgDn";       break;
    case Key::Home:         os << "Home";       break;
    case Key::End:          os << "End";        break;
    case Key::CapsLock:     os << "CapsLock";   break;
    case Key::ScrollLock:   os << "ScrollLock"; break;
    case Key::NumLock:      os << "NumLock";    break;
    case Key::PrintScreen:  os << "PrintScrn";  break;
    case Key::Pause:        os << "Pause";      break;
    case Key::F1:           os << "F1";         break;
    case Key::F2:           os << "F2";         break;
    case Key::F3:           os << "F3";         break;
    case Key::F4:           os << "F4";         break;
    case Key::F5:           os << "F5";         break;
    case Key::F6:           os << "F6";         break;
    case Key::F7:           os << "F7";         break;
    case Key::F8:           os << "F8";         break;
    case Key::F9:           os << "F9";         break;
    case Key::F10:          os << "F10";        break;
    case Key::F11:          os << "F11";        break;
    case Key::F12:          os << "F12";        break;
    case Key::F13:          os << "F13";        break;
    case Key::F14:          os << "F14";        break;
    case Key::F15:          os << "F15";        break;
    case Key::F16:          os << "F16";        break;
    case Key::F17:          os << "F17";        break;
    case Key::F18:          os << "F18";        break;
    case Key::F19:          os << "F19";        break;
    case Key::F20:          os << "F20";        break;
    case Key::F21:          os << "F21";        break;
    case Key::F22:          os << "F22";        break;
    case Key::F23:          os << "F23";        break;
    case Key::F24:          os << "F24";        break;
    case Key::F25:          os << "F25";        break;
    case Key::Num0:         os << "Num0";       break;
    case Key::Num1:         os << "Num1";       break;
    case Key::Num2:         os << "Num2";       break;
    case Key::Num3:         os << "Num3";       break;
    case Key::Num4:         os << "Num4";       break;
    case Key::Num5:         os << "Num5";       break;
    case Key::Num6:         os << "Num6";       break;
    case Key::Num7:         os << "Num7";       break;
    case Key::Num8:         os << "Num8";       break;
    case Key::Num9:         os << "Num9";       break;
    case Key::NumDecimal:   os << "NumDecimal"; break;
    case Key::NumDivide:    os << "NumDiv";     break;
    case Key::NumMultiply:  os << "NumMul";     break;
    case Key::NumSubtract:  os << "NumSub";     break;
    case Key::NumAdd:       os << "NumAdd";     break;
    case Key::NumEnter:     os << "NumEnter";   break;
    case Key::NumEqual:     os << "NumEqual";   break;
    case Key::LeftShift:    os << "Shift";      break;
    case Key::LeftCtrl:     os << "Ctrl";       break;
    case Key::LeftAlt:      os << "Alt";        break;
    case Key::LeftSuper:    os << "Super";      break;
    case Key::RightShift:   os << "RShift";     break;
    case Key::RightCtrl:    os << "RCtrl";      break;
    case Key::RightAlt:     os << "RAlt";       break;
    case Key::RightSuper:   os << "RSuper";     break;
    case Key::Menu:         os << "Menu";       break;
    }
    os << '>';
    return os;
}

std::ostream& operator<<(std::ostream& os, ModifierFlag const& m)
{
    bool is_first = true;
    if ((static_cast<int>(m) & static_cast<int>(ModifierFlag::Shift)) != 0) {
        os << Key::LeftShift;
        is_first = false;
    }
    if ((static_cast<int>(m) & static_cast<int>(ModifierFlag::Ctrl)) != 0) {
        if (!is_first) { os << '+'; }
        os << Key::LeftCtrl;
        is_first = false;
    }
    if ((static_cast<int>(m) & static_cast<int>(ModifierFlag::Alt)) != 0) {
        if (!is_first) { os << '+'; }
        os << Key::LeftAlt;
        is_first = false;
    }
    if ((static_cast<int>(m) & static_cast<int>(ModifierFlag::Super)) != 0) {
        if (!is_first) { os << '+'; }
        os << Key::LeftSuper;
        is_first = false;
    }
    if ((static_cast<int>(m) & static_cast<int>(ModifierFlag::CapsLock)) != 0) {
        if (!is_first) { os << '+'; }
        os << Key::CapsLock;
        is_first = false;
    }
    if ((static_cast<int>(m) & static_cast<int>(ModifierFlag::NumLock)) != 0) {
        if (!is_first) { os << '+'; }
        os << Key::NumLock;
        is_first = false;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, KeyAction const& a)
{
    switch (a) {
    case KeyAction::Press:      os << "Press";      break;
    case KeyAction::Release:    os << "Release";    break;
    case KeyAction::Repeat:     os << "Repeat";     break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, MouseButton const& b)
{
    switch (b) {
    case MouseButton::Left:     os << "MouseL"; break;
    case MouseButton::Right:    os << "MouseR"; break;
    case MouseButton::Middle:   os << "MouseM"; break;
    case MouseButton::Button4:  os << "Mouse4"; break;
    case MouseButton::Button5:  os << "Mouse5"; break;
    case MouseButton::Button6:  os << "Mouse6"; break;
    case MouseButton::Button7:  os << "Mouse7"; break;
    case MouseButton::Button8:  os << "Mouse8"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, MouseButtonAction const& a)
{
    switch (a) {
    case MouseButtonAction::Press:      os << "Press";      break;
    case MouseButtonAction::Release:    os << "Release";    break;
    }
    return os;
}


}
