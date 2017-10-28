#include <central_widget.hpp>

#include <gbVk/Instance.hpp>
#include <gbVk/PhysicalDevice.hpp>
#include <gbVk/StringConverters.hpp>

#include <QStringBuilder>

namespace Ui
{
/*QVBoxLayout m_instancePaneLayout;
    QLabel* m_layersLabel;
    QListWidget* m_layersList;
    QLabel* m_extensionsLabel;
    QListWidget* m_extensionsList;*/

CentralWidget::CentralWidget(QWidget* parent)
    :QWidget(parent),
     m_layersLabel(new QLabel("Instance Layers", this)), m_layersList(new QListWidget(this)),
     m_layersDescription(new QTextEdit(this)),
     m_extensionsLabel(new QLabel("Instance Extensions", this)), m_extensionsList(new QListWidget(this)),
     m_extensionsDescription(new QTextEdit(this)),
     m_physicalDevices(new QListWidget(this)), m_instance(GhulbusVulkan::Instance::createInstance())
{
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp.setVerticalStretch(2);

    m_instancePaneLayout.addWidget(m_layersLabel);
    m_layersList->setSizePolicy(sp);
    m_instancePaneLayout.addWidget(m_layersList);
    m_instancePaneLayout.addWidget(m_layersDescription);
    m_layersDescription->setEnabled(false);
    sp.setVerticalStretch(1);
    m_layersDescription->setSizePolicy(sp);
    m_instancePaneLayout.addWidget(m_extensionsLabel);
    sp.setVerticalStretch(2);
    m_extensionsList->setSizePolicy(sp);
    m_instancePaneLayout.addWidget(m_extensionsList);
    m_extensionsDescription->setEnabled(false);
    sp.setVerticalStretch(1);
    m_extensionsDescription->setSizePolicy(sp);
    m_instancePaneLayout.addWidget(m_extensionsDescription);
    m_layout.addLayout(&m_instancePaneLayout);

    m_layout.addWidget(m_physicalDevices);

    setLayout(&m_layout);

    connect(m_layersList, &QListWidget::currentRowChanged,
            this, &CentralWidget::onInstanceLayerSelected);
    connect(m_extensionsList, &QListWidget::currentRowChanged,
            this, &CentralWidget::onInstanceExtensionSelected);
}

void CentralWidget::enumerateInstanceOptions()
{
    auto const layer_props = GhulbusVulkan::Instance::enumerateInstanceLayerProperties();
    m_layersDescriptionStrings.clear();
    for(auto const& p : layer_props) {
        auto item = new QListWidgetItem(QString(p.layerName));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        m_layersDescriptionStrings.emplace_back(
            QString(p.description) %
            "\n\nSpec Version: " % QString::fromStdString(GhulbusVulkan::version_to_string(p.specVersion)) %
            "\nImplementation Version: " % QString::fromStdString(GhulbusVulkan::version_to_string(p.implementationVersion))
        );
        m_layersList->addItem(item);
    }

    auto const extension_props = GhulbusVulkan::Instance::enumerateInstanceExtensionProperties();
    m_extensionsDescriptionStrings.clear();
    for(auto const& p : extension_props) {
        auto item = new QListWidgetItem(QString(p.extensionName));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        m_extensionsDescriptionStrings.emplace_back(
            QString("Spec Version: ") % QString::fromStdString(GhulbusVulkan::version_to_string(p.specVersion))
        );
        m_extensionsList->addItem(item);
    }
}

void CentralWidget::enumeratePhysicalDevices()
{
}

void CentralWidget::onInstanceLayerSelected(int index)
{
    if(index < 0 || index >= m_layersDescriptionStrings.size()) {
        m_layersDescription->setText("");
    } else {
        m_layersDescription->setText(m_layersDescriptionStrings[index]);
    }
}

void CentralWidget::onInstanceExtensionSelected(int index)
{
    if(index < 0 || index >= m_extensionsDescriptionStrings.size()) {
        m_extensionsDescription->setText("");
    } else {
        m_extensionsDescription->setText(m_extensionsDescriptionStrings[index]);
    }
}

}
