#include <main_window.hpp>

#include <central_widget.hpp>

namespace Ui
{

MainWindow::MainWindow(QWidget* parent)
    :QMainWindow(parent), m_centralWidget(new CentralWidget(this))
{
    setCentralWidget(m_centralWidget);

    m_centralWidget->enumerateInstanceOptions();
    m_centralWidget->onInstanceConfigurationChanged();
}

}
