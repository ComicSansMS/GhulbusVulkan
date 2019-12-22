#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_WINDOW_EVENTS_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_WINDOW_EVENTS_HPP

/** @file
*
* @brief Window Events.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbMath/Vector2.hpp>

#include <cstdint>
#include <iosfwd>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
enum class Key {
    Unknown         =  -1,
    Space           =  32,
    Apostrophe      =  39,  /* ' */
    Comma           =  44,  /* , */
    Minus           =  45,  /* - */
    Period          =  46,  /* . */
    Slash           =  47,  /* / */
    k0              =  48,
    k1              =  49,
    k2              =  50,
    k3              =  51,
    k4              =  52,
    k5              =  53,
    k6              =  54,
    k7              =  55,
    k8              =  56,
    k9              =  57,
    Semicolon       =  59,  /* ; */
    Equal           =  61,  /* = */
    A               =  65,
    B               =  66,
    C               =  67,
    D               =  68,
    E               =  69,
    F               =  70,
    G               =  71,
    H               =  72,
    I               =  73,
    J               =  74,
    K               =  75,
    L               =  76,
    M               =  77,
    N               =  78,
    O               =  79,
    P               =  80,
    Q               =  81,
    R               =  82,
    S               =  83,
    T               =  84,
    U               =  85,
    V               =  86,
    W               =  87,
    X               =  88,
    Y               =  89,
    Z               =  90,
    LeftBracket     =  91,  /* [ */
    Backslash       =  92,  /* \ */
    RightBracket    =  93,  /* ] */
    GraveAccent     =  96,  /* ` */
    World1          = 161, /* non-US #1 */
    World2          = 162, /* non-US #2 */

    Escape          = 256,
    Enter           = 257,
    Tab             = 258,
    Backspace       = 259,
    Insert          = 260,
    Delete          = 261,
    Right           = 262,
    Left            = 263,
    Down            = 264,
    Up              = 265,
    PageUp          = 266,
    PageDown        = 267,
    Home            = 268,
    End             = 269,
    CapsLock        = 280,
    ScrollLock      = 281,
    NumLock         = 282,
    PrintScreen     = 283,
    Pause           = 284,
    F1              = 290,
    F2              = 291,
    F3              = 292,
    F4              = 293,
    F5              = 294,
    F6              = 295,
    F7              = 296,
    F8              = 297,
    F9              = 298,
    F10             = 299,
    F11             = 300,
    F12             = 301,
    F13             = 302,
    F14             = 303,
    F15             = 304,
    F16             = 305,
    F17             = 306,
    F18             = 307,
    F19             = 308,
    F20             = 309,
    F21             = 310,
    F22             = 311,
    F23             = 312,
    F24             = 313,
    F25             = 314,
    Num0            = 320,
    Num1            = 321,
    Num2            = 322,
    Num3            = 323,
    Num4            = 324,
    Num5            = 325,
    Num6            = 326,
    Num7            = 327,
    Num8            = 328,
    Num9            = 329,
    NumDecimal      = 330,
    NumDivide       = 331,
    NumMultiply     = 332,
    NumSubtract     = 333,
    NumAdd          = 334,
    NumEnter        = 335,
    NumEqual        = 336,
    LeftShift       = 340,
    LeftCtrl        = 341,
    LeftAlt         = 342,
    LeftSuper       = 343,
    RightShift      = 344,
    RightCtrl       = 345,
    RightAlt        = 346,
    RightSuper      = 347,
    Menu            = 348
};

std::ostream& operator<<(std::ostream& os, Key const& k);

enum class ModifierFlag {
    Shift       = 0x0001,
    Ctrl        = 0x0002,
    Alt         = 0x0004,
    Super       = 0x0008,
    CapsLock    = 0x0010,
    NumLock     = 0x0020
};

std::ostream& operator<<(std::ostream& os, ModifierFlag const& m);

enum class KeyAction {
    Release     = 0,
    Press       = 1,
    Repeat      = 2
};

std::ostream& operator<<(std::ostream& os, KeyAction const& a);

enum class KeyState {
    Released    = 0,
    Pressed     = 1
};

std::ostream& operator<<(std::ostream& os, KeyState const& s);

enum class MouseButton {
    Left        = 0,
    Right       = 1,
    Middle      = 2,
    Button4     = 3,
    Button5     = 4,
    Button6     = 5,
    Button7     = 6,
    Button8     = 7,
};

std::ostream& operator<<(std::ostream& os, MouseButton const& b);

enum class MouseButtonAction {
    Release = 0,
    Press = 1
};

std::ostream& operator<<(std::ostream& os, MouseButtonAction const& a);

namespace Event {
struct Key {
    GHULBUS_GRAPHICS_NAMESPACE::Key key;
    KeyAction action;
    ModifierFlag modifiers;
};

struct Text {
    char32_t codepoint;
};

struct MouseMove {
    GhulbusMath::Vector2d position;
};

struct MouseLeave {};
struct MouseEnter {};

struct MouseClick {
    MouseButton button;
    MouseButtonAction action;
    ModifierFlag modifiers;
};

struct MouseScroll {
    GhulbusMath::Vector2d offset;
};

struct ViewportResize {
    uint32_t new_width;
    uint32_t new_height;
};
}
}
#endif
