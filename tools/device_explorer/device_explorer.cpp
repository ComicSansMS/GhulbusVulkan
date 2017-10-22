
#include <QApplication>

#include <gbVk/Instance.hpp>

#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>
#include <gbBase/LogHandlers.hpp>

#include <main_window.hpp>


int main(int argc, char* argv[])
{
    QApplication the_app(argc, argv);

    Ghulbus::Log::initializeLogging();
    auto const finalize_logging = Ghulbus::finally([]() { Ghulbus::Log::shutdownLogging(); });

    Ghulbus::Log::Handlers::LogToFile file_logger("device_explorer.log");
#if defined WIN32 && !defined NDEBUG
    Ghulbus::Log::Handlers::LogMultiSink multisink_logger(Ghulbus::Log::Handlers::logToWindowsDebugger, file_logger);
    Ghulbus::Log::Handlers::LogAsync handler(multisink_logger);
#else
    Ghulbus::Log::Handlers::LogAsync handler(file_logger);
#endif
    Ghulbus::Log::setLogHandler(handler);
    handler.start();
    auto const handler_join = Ghulbus::finally([&]() { handler.stop(); });
#ifdef NDEBUG
    Ghulbus::Log::setLogLevel(Ghulbus::LogLevel::Info);
#else
    Ghulbus::Log::setLogLevel(Ghulbus::LogLevel::Trace);
#endif
    GHULBUS_LOG(Info, "Device Explorer starting...");

    auto const layers = GhulbusVulkan::Instance::enumerateInstanceLayerProperties();
    std::vector<std::vector<VkExtensionProperties>> extension_props;
    for(auto const& l : layers) {
        extension_props.emplace_back(GhulbusVulkan::Instance::enumerateInstanceExtensionProperties(l));
    }
    std::vector<VkExtensionProperties> instance_extension_props = GhulbusVulkan::Instance::enumerateInstanceExtensionProperties();
    auto instance = GhulbusVulkan::Instance::createInstance();


    Ui::MainWindow main_window;
    main_window.setWindowTitle("gbVk Device Explorer");
    main_window.show();

    return the_app.exec();
}
