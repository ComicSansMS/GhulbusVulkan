#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_REACTOR_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_REACTOR_HPP

/** @file
*
* @brief Reactor.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <functional>
#include <vector>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;
class Window;

class Reactor {
public:
    using Work = std::function<void()>;
private:
    GraphicsInstance* m_instance;
    std::vector<Work> m_work;
public:
    Reactor(GraphicsInstance& instance);

    void post(Work w);

    void pump();
};

}
#endif
