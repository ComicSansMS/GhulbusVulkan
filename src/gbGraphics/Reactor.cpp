#include <gbGraphics/Reactor.hpp>

#include <gbGraphics/GraphicsInstance.hpp>
#include <gbGraphics/Window.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

Reactor::Reactor(GraphicsInstance& instance)
    :m_instance(&instance)
{
}

void Reactor::post(Work w)
{
    m_work.emplace_back(std::move(w));
}

void Reactor::pump()
{
    m_instance->pollEvents();
    for(auto const& w : m_work) {
        w();
    }
    m_work.clear();
}

}
