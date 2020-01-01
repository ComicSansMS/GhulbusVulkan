
#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>
#include <gbBase/LogHandlers.hpp>
#include <gbBase/UnusedVariable.hpp>

#include <gbGraphics/Graphics.hpp>

#include <gbBase/PerfLog.hpp>

int main()
{
    auto const gblog_init_guard = Ghulbus::Log::initializeLoggingWithGuard();
    Ghulbus::Log::Handlers::LogSynchronizeMutex logger(Ghulbus::Log::Handlers::logToCout);
    Ghulbus::Log::setLogHandler(logger);
    Ghulbus::Log::setLogLevel(Ghulbus::LogLevel::Trace);

    Ghulbus::PerfLog perflog;

    GhulbusGraphics::GraphicsInstance graphics_instance;
    perflog.tick(Ghulbus::LogLevel::Debug, "gbGraphics Init");

    int const WINDOW_WIDTH = 1280;
    int const WINDOW_HEIGHT = 720;
    GhulbusGraphics::Window main_window(graphics_instance, WINDOW_WIDTH, WINDOW_HEIGHT, u8"Vulkan 2d Demo");
    GhulbusGraphics::Draw2d draw2d(graphics_instance, main_window);

    perflog.tick(Ghulbus::LogLevel::Debug, "Window creation");

    while(!main_window.isDone()) {
        graphics_instance.pollEvents();

        draw2d.draw();
    }
}
