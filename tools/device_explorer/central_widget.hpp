#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVEXPLORER_CENTRAL_WIDGET_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVEXPLORER_CENTRAL_WIDGET_HPP

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
#include <QWidget>

#include <gbVk/Instance.hpp>

namespace Ui
{

class CentralWidget : public QWidget
{
    Q_OBJECT
private:
    QHBoxLayout m_layout;

    QVBoxLayout m_instancePaneLayout;
    QLabel* m_layersLabel;
    QListWidget* m_layersList;
    QTextEdit* m_layersDescription;
    std::vector<QString> m_layersDescriptionStrings;
    QLabel* m_extensionsLabel;
    QListWidget* m_extensionsList;
    std::vector<QString> m_extensionsDescriptionStrings;
    QTextEdit* m_extensionsDescription;

    QListWidget* m_physicalDevices;

    GhulbusVulkan::Instance m_instance;
public:
    CentralWidget(QWidget* parent);

public slots:
    void enumerateInstanceOptions();
    void enumeratePhysicalDevices();
private slots:
    void onInstanceLayerSelected(int index);
    void onInstanceExtensionSelected(int index);

};

}

#endif
