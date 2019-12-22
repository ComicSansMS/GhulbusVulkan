#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_WINDOW_EVENT_REACTOR_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_WINDOW_EVENT_REACTOR_HPP

/** @file
*
* @brief Window Event Reactor.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/WindowEvents.hpp>

#include <gbBase/AnyInvocable.hpp>
#include <gbBase/Assert.hpp>

#include <algorithm>
#include <functional>
#include <vector>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;
class Window;

struct WindowResizeEvent {
    int width;
    int height;
};

class WindowEventReactor {
public:
    enum class Result {
        ConsumeEvent,
        ContinueProcessing
    };

    using KeyEventHandler               = Ghulbus::AnyInvocable<Result(Event::Key const&)>;
    using TextEventHandler              = Ghulbus::AnyInvocable<Result(Event::Text const&)>;
    using MouseMoveEventHandler         = Ghulbus::AnyInvocable<Result(Event::MouseMove const&)>;
    using MouseLeaveEventHandler        = Ghulbus::AnyInvocable<Result(Event::MouseLeave const&)>;
    using MouseEnterEventHandler        = Ghulbus::AnyInvocable<Result(Event::MouseEnter const&)>;
    using MouseClickEventHandler        = Ghulbus::AnyInvocable<Result(Event::MouseClick const&)>;
    using MouseScrollEventHandler       = Ghulbus::AnyInvocable<Result(Event::MouseScroll const&)>;
    using ViewportResizeEventHandler    = Ghulbus::AnyInvocable<Result(Event::ViewportResize const&)>;

    template<typename Handler_T>
    class HandlerContainer {
    public:
        class [[nodiscard]] Guard {
            friend class HandlerContainer;
        private:
            HandlerContainer* m_parent;
            size_t m_handlerId;
        private:
            Guard(HandlerContainer& parent, size_t handler_id)
                :m_parent(&parent), m_handlerId(handler_id)
            {}
        public:
            ~Guard()
            {
                cleanup();
            }

            Guard(Guard&& rhs) noexcept
                :m_parent(rhs.m_parent), m_handlerId(rhs.m_handlerId)
            {
                rhs.m_parent = nullptr;
                rhs.m_handlerId = static_cast<size_t>(-1);
            }

            Guard& operator=(Guard&& rhs) noexcept
            {
                if (&rhs != this) {
                    cleanup();
                    m_parent = rhs.m_parent;
                    m_handlerId = rhs.m_handlerId;
                    rhs.m_parent = nullptr;
                    rhs.m_handlerId = 0;
                }
                return *this;
            }
        private:
            void cleanup() noexcept
            {
                if (m_parent) {
                    m_parent->removeHandler(m_handlerId);
                }
            }
        };
    private:
        std::vector<Handler_T> m_handlers;
        std::vector<size_t> m_handlerIds;
        size_t m_currentIdCount = 1;
    public:
        HandlerContainer() = default;
        ~HandlerContainer() = default;
        HandlerContainer(HandlerContainer const&) = delete;
        HandlerContainer& operator=(HandlerContainer const&) = delete;

        Guard addHandler(Handler_T const& event_handler)
        {
            m_handlers.insert(m_handlers.begin(), event_handler);
            auto const it_id = m_handlerIds.insert(m_handlerIds.begin(), m_currentIdCount++);
            GHULBUS_ASSERT(m_handlers.size() == m_handlerIds.size());
            return Guard(*this, *it_id);
        }

        Guard addHandler(Handler_T&& event_handler)
        {
            m_handlers.emplace(m_handlers.begin(), std::move(event_handler));
            auto const it_id = m_handlerIds.insert(m_handlerIds.begin(), m_currentIdCount++);
            GHULBUS_ASSERT(m_handlers.size() == m_handlerIds.size());
            return Guard(*this, *it_id);
        }

        template<typename... T_Args>
        void invokeHandlers(T_Args const&... args)
        {
            for(auto const& f : m_handlers) {
                Result const res = f(args...);
                if(res == Result::ConsumeEvent) { break; }
            }
        }

    private:
        void removeHandler(size_t id) noexcept
        {
            auto const it_id = std::find(m_handlerIds.begin(), m_handlerIds.end(), id);
            GHULBUS_ASSERT(it_id != m_handlerIds.end());
            size_t const index = std::distance(m_handlerIds.begin(), it_id);
            auto const it_handler = m_handlers.begin() + index;
            m_handlers.erase(it_handler);
            m_handlerIds.erase(it_id);
        }
    };

    struct Handlers {
        HandlerContainer<KeyEventHandler> keyEvent;
        HandlerContainer<TextEventHandler> textEvent;
        HandlerContainer<MouseMoveEventHandler> mouseMoveEvent;
        HandlerContainer<MouseLeaveEventHandler> mouseLeaveEvent;
        HandlerContainer<MouseEnterEventHandler> mouseEnterEvent;
        HandlerContainer<MouseClickEventHandler> mouseClickEvent;
        HandlerContainer<MouseScrollEventHandler> mouseScrollEvent;
        HandlerContainer<ViewportResizeEventHandler> viewportResizeEvent;
    } eventHandlers;
public:
    WindowEventReactor();

    void onKey(Event::Key const& e);
    void onText(Event::Text const& e);
    void onMouseMove(Event::MouseMove const& e);
    void onMouseEnter(Event::MouseEnter const& e);
    void onMouseLeave(Event::MouseLeave const& e);
    void onMouseClick(Event::MouseClick const& e);
    void onMouseScroll(Event::MouseScroll const& e);
    void onViewportResize(Event::ViewportResize const& e);
};

}
#endif
