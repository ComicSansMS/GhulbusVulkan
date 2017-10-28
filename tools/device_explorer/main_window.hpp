#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVEXPLORER_MAIN_WINDOW_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVEXPLORER_MAIN_WINDOW_HPP

#include <QMainWindow>
namespace Ui
{

class CentralWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    CentralWidget* m_centralWidget;
public:
    MainWindow(QWidget* parent = nullptr);
};

}
#endif
