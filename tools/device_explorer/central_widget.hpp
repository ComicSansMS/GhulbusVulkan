#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVEXPLORER_CENTRAL_WIDGET_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVEXPLORER_CENTRAL_WIDGET_HPP

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
#include <QTreeWidget>
#include <QWidget>

#include <gbVk/PhysicalDevice.hpp>

#include <memory>

namespace GhulbusVulkan {
class Instance;
}

namespace Ui
{

class CentralWidget : public QWidget
{
    Q_OBJECT
private:
    QVBoxLayout m_layout;

    QHBoxLayout m_innerLayout;

    QVBoxLayout m_instancePaneLayout;
    QLabel* m_layersLabel;
    QListWidget* m_layersList;
    QTextEdit* m_layersDescription;
    std::vector<QString> m_layersDescriptionStrings;
    QLabel* m_extensionsLabel;
    QListWidget* m_extensionsList;
    std::vector<QString> m_extensionsDescriptionStrings;
    QTextEdit* m_extensionsDescription;

    QVBoxLayout m_physicalDevicesPaneLayout;
    QLabel* m_physicalDevicesLabel;
    QListWidget* m_physicalDevicesList;
    std::vector<GhulbusVulkan::PhysicalDevice> m_physicalDevices;
    QTextEdit* m_physicalDeviceDescription;
    QTreeWidget* m_physicalDeviceProperties;

    QTextEdit* m_errorConsole;

    std::unique_ptr<GhulbusVulkan::Instance> m_instance;
public:
    CentralWidget(QWidget* parent);

    ~CentralWidget();

signals:
    void errorCreateInstance(QString);
    void newInstanceCreated();

public slots:
    void logError(QString msg);
    void enumerateInstanceOptions();
    void enumeratePhysicalDevices();
    void onInstanceConfigurationChanged();

private slots:
    void onInstanceLayerSelected(int index);
    void onInstanceExtensionSelected(int index);
    void onPhysicalDeviceSelected(int index);

private:
    void connectInstanceLayerListSignals();
    void connectInstanceExtensionListSignals();

};

}

#endif
