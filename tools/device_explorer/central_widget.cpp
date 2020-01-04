#include <central_widget.hpp>

#include <gbBase/Assert.hpp>

#include <gbVk/Exceptions.hpp>
#include <gbVk/Instance.hpp>
#include <gbVk/PhysicalDevice.hpp>
#include <gbVk/StringConverters.hpp>

#include <QStringBuilder>

#include <sstream>

namespace {
QList<QTreeWidgetItem*> buildLimitsList(VkPhysicalDeviceLimits limits);
}

namespace Ui
{

CentralWidget::CentralWidget(QWidget* parent)
    :QWidget(parent),
     m_layersLabel(new QLabel("Instance Layers", this)), m_layersList(new QListWidget(this)),
     m_layersDescription(new QTextEdit(this)),
     m_extensionsLabel(new QLabel("Instance Extensions", this)), m_extensionsList(new QListWidget(this)),
     m_extensionsDescription(new QTextEdit(this)),
     m_physicalDevicesLabel(new QLabel("Physical Devices", this)), m_physicalDevicesList(new QListWidget(this)),
     m_physicalDeviceDescription(new QTextEdit(this)), m_physicalDeviceProperties(new QTreeWidget(this)),
     m_errorConsole(new QTextEdit(this)), m_instance(nullptr)
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
    m_innerLayout.addLayout(&m_instancePaneLayout);

    m_physicalDevicesPaneLayout.addWidget(m_physicalDevicesLabel);
    m_physicalDevicesPaneLayout.addWidget(m_physicalDevicesList);
    m_physicalDeviceDescription->setReadOnly(true);
    m_physicalDevicesPaneLayout.addWidget(m_physicalDeviceDescription);
    m_physicalDevicesPaneLayout.addWidget(m_physicalDeviceProperties);
    m_innerLayout.addLayout(&m_physicalDevicesPaneLayout);

    m_layout.addLayout(&m_innerLayout);

    sp.setVerticalPolicy(QSizePolicy::Fixed);
    m_errorConsole->setMinimumHeight(50);
    m_errorConsole->setSizePolicy(sp);
    m_errorConsole->setReadOnly(true);
    m_layout.addWidget(m_errorConsole);

    setLayout(&m_layout);

    connect(this, &CentralWidget::errorCreateInstance,
            this, &CentralWidget::logError);

    connect(this, &CentralWidget::newInstanceCreated,
            this, &CentralWidget::enumeratePhysicalDevices);

    connect(m_physicalDevicesList, &QListWidget::currentRowChanged,
            this, &CentralWidget::onPhysicalDeviceSelected);

    connectInstanceLayerListSignals();
    connectInstanceExtensionListSignals();
}

CentralWidget::~CentralWidget() = default;

void CentralWidget::connectInstanceLayerListSignals()
{
    connect(m_layersList, &QListWidget::currentRowChanged,
            this, &CentralWidget::onInstanceLayerSelected);
    connect(m_layersList, &QListWidget::itemChanged,
            this, &CentralWidget::onInstanceConfigurationChanged);
}

void CentralWidget::connectInstanceExtensionListSignals()
{
    connect(m_extensionsList, &QListWidget::currentRowChanged,
            this, &CentralWidget::onInstanceExtensionSelected);
    connect(m_extensionsList, &QListWidget::itemChanged,
            this, &CentralWidget::onInstanceConfigurationChanged);
}

void CentralWidget::enumerateInstanceOptions()
{
    disconnect(m_layersList, nullptr, nullptr, nullptr);
    m_layersList->clear();
    m_layersDescriptionStrings.clear();
    m_layersDescription->clear();

    auto const layer_props = GhulbusVulkan::Instance::enumerateInstanceLayerProperties();
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
    onInstanceLayerSelected(0);
    connectInstanceLayerListSignals();

    disconnect(m_extensionsList, nullptr, nullptr, nullptr);
    m_extensionsList->clear();
    m_extensionsDescriptionStrings.clear();
    m_extensionsDescription->clear();

    auto const extension_props = GhulbusVulkan::Instance::enumerateInstanceExtensionProperties();
    for(auto const& p : extension_props) {
        auto item = new QListWidgetItem(QString(p.extensionName));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        m_extensionsDescriptionStrings.emplace_back(
            QString("Spec Version: ") % QString::fromStdString(GhulbusVulkan::version_to_string(p.specVersion))
        );
        m_extensionsList->addItem(item);
    }
    onInstanceExtensionSelected(0);
    connectInstanceExtensionListSignals();
}

void CentralWidget::enumeratePhysicalDevices()
{
    m_physicalDevices.clear();
    m_physicalDevicesList->clear();
    m_physicalDevicesList->setEnabled(false);
    if(!m_instance) { return; }

    m_physicalDevices = m_instance->enumeratePhysicalDevices();
    for(auto& pd : m_physicalDevices) {
        m_physicalDevicesList->addItem(QString(pd.getProperties().deviceName));
    }
    m_physicalDevicesList->setEnabled(true);
}

void CentralWidget::logError(QString msg)
{
    m_errorConsole->append(msg);
}

void CentralWidget::onInstanceLayerSelected(int index)
{
    if(index < 0 || index >= m_layersDescriptionStrings.size()) {
        m_layersDescription->clear();
    } else {
        m_layersDescription->setText(m_layersDescriptionStrings[index]);
    }
}

void CentralWidget::onInstanceExtensionSelected(int index)
{
    if(index < 0 || index >= m_extensionsDescriptionStrings.size()) {
        m_extensionsDescription->clear();
    } else {
        m_extensionsDescription->setText(m_extensionsDescriptionStrings[index]);
    }
}

void CentralWidget::onPhysicalDeviceSelected(int index)
{
    if(index < 0 || index >= m_physicalDevices.size()) {
        m_physicalDeviceDescription->clear();
        m_physicalDeviceProperties->clear();
    } else {
        auto& pd = m_physicalDevices[index];
        auto props = pd.getProperties();
        std::stringstream sstr;
        sstr << props.deviceName << " (" << props.deviceType << ")\n\n"
             << "Vendor ID: " << props.vendorID << "\n"
             << "Device ID: " << props.deviceID << "\n"
             << "Pipeline Cache UUID: " << GhulbusVulkan::uuid_to_string(props.pipelineCacheUUID) << "\n"
             << "API Version: " << GhulbusVulkan::version_to_string(props.apiVersion) << "\n"
             << "Driver Version: " << GhulbusVulkan::version_to_string(props.driverVersion);
        m_physicalDeviceDescription->setText(QString::fromStdString(sstr.str()));

        m_physicalDeviceProperties->setColumnCount(2);
        m_physicalDeviceProperties->addTopLevelItems(buildLimitsList(props.limits));
    }
}

void CentralWidget::onInstanceConfigurationChanged()
{
    m_instance.reset();

    GhulbusVulkan::Instance::Layers layers;
    std::vector<std::string> layer_storage;
    for(int i = 0; i < m_layersList->count(); ++i) {
        auto item = m_layersList->item(i);
        if(item->checkState() == Qt::Checked) {
            layer_storage.emplace_back(item->text().toStdString());
        }
    }
    for(auto const& l :layer_storage) { layers.addLayer(l.c_str()); }
    GhulbusVulkan::Instance::Extensions extensions(GhulbusVulkan::Instance::Extensions::DeactivateSurfaceExtensions{});
    std::vector<std::string> extensions_storage;
    for(int i = 0; i < m_extensionsList->count(); ++i) {
        auto item = m_extensionsList->item(i);
        if(item->checkState() == Qt::Checked) {
            extensions_storage.emplace_back(item->text().toStdString());
        }
    }
    for(auto const& e : extensions_storage) { extensions.addExtension(e.c_str()); }

    try {
        m_instance = std::make_unique<GhulbusVulkan::Instance>(
            GhulbusVulkan::Instance::createInstance(
                "gbVk Device Explorer", GhulbusVulkan::Instance::Version(1, 0, 0), layers, extensions));
    } catch(GhulbusVulkan::Exceptions::VulkanError& e) {
        auto desc = Ghulbus::getErrorInfo<GhulbusVulkan::Exception_Info::description>(e);
        auto vkres = Ghulbus::getErrorInfo<GhulbusVulkan::Exception_Info::vulkan_error_code>(e);
        emit errorCreateInstance(QString::fromStdString(*desc) % " - " % GhulbusVulkan::to_string(*vkres));
        return;
    }
    emit newInstanceCreated();
}
}

namespace {
QList<QTreeWidgetItem*> buildLimitsList(VkPhysicalDeviceLimits limits)
{
    QList<QTreeWidgetItem*> ret;
    auto add_item = [&ret](char const* label, std::string const& value) {
        QStringList qsl;
        qsl.push_back(QString::fromUtf8(label));
        qsl.push_back(QString::fromStdString(value));
        ret.push_back(new QTreeWidgetItem(qsl));
    };
#define ADD_ITEM(field) add_item(#field, std::to_string(limits.field))
#define ADD_ITEM_SIZE(field) add_item(#field, GhulbusVulkan::memory_size_to_string(limits.field))
    ADD_ITEM(maxImageDimension1D);
    ADD_ITEM(maxImageDimension2D);
    ADD_ITEM(maxImageDimension3D);
    ADD_ITEM(maxImageDimensionCube);
    ADD_ITEM(maxImageArrayLayers);
    ADD_ITEM(maxTexelBufferElements);
    ADD_ITEM(maxUniformBufferRange);
    ADD_ITEM(maxStorageBufferRange);
    ADD_ITEM(maxPushConstantsSize);
    ADD_ITEM(maxMemoryAllocationCount);
    ADD_ITEM(maxSamplerAllocationCount);
    ADD_ITEM_SIZE(bufferImageGranularity);
    ADD_ITEM_SIZE(sparseAddressSpaceSize);
    ADD_ITEM(maxBoundDescriptorSets);
    ADD_ITEM(maxPerStageDescriptorSamplers);
    ADD_ITEM(maxPerStageDescriptorUniformBuffers);
    ADD_ITEM(maxPerStageDescriptorStorageBuffers);
    ADD_ITEM(maxPerStageDescriptorSampledImages);
    ADD_ITEM(maxPerStageDescriptorStorageImages);
    ADD_ITEM(maxPerStageDescriptorInputAttachments);
    ADD_ITEM(maxPerStageResources);
    ADD_ITEM(maxDescriptorSetSamplers);
    ADD_ITEM(maxDescriptorSetUniformBuffers);
    ADD_ITEM(maxDescriptorSetUniformBuffersDynamic);
    ADD_ITEM(maxDescriptorSetStorageBuffers);
    ADD_ITEM(maxDescriptorSetStorageBuffersDynamic);
    ADD_ITEM(maxDescriptorSetSampledImages);
    ADD_ITEM(maxDescriptorSetStorageImages);
    ADD_ITEM(maxDescriptorSetInputAttachments);
    ADD_ITEM(maxVertexInputAttributes);
    ADD_ITEM(maxVertexInputBindings);
    ADD_ITEM(maxVertexInputAttributeOffset);
    ADD_ITEM(maxVertexInputBindingStride);
    ADD_ITEM(maxVertexOutputComponents);
    ADD_ITEM(maxTessellationGenerationLevel);
    ADD_ITEM(maxTessellationPatchSize);
    ADD_ITEM(maxTessellationControlPerVertexInputComponents);
    ADD_ITEM(maxTessellationControlPerVertexOutputComponents);
    ADD_ITEM(maxTessellationControlPerPatchOutputComponents);
    ADD_ITEM(maxTessellationControlTotalOutputComponents);
    ADD_ITEM(maxTessellationEvaluationInputComponents);
    ADD_ITEM(maxTessellationEvaluationOutputComponents);
    ADD_ITEM(maxGeometryShaderInvocations);
    ADD_ITEM(maxGeometryInputComponents);
    ADD_ITEM(maxGeometryOutputComponents);
    ADD_ITEM(maxGeometryOutputVertices);
    ADD_ITEM(maxGeometryTotalOutputComponents);
    ADD_ITEM(maxFragmentInputComponents);
    ADD_ITEM(maxFragmentOutputAttachments);
    ADD_ITEM(maxFragmentDualSrcAttachments);
    ADD_ITEM(maxFragmentCombinedOutputResources);
    ADD_ITEM(maxComputeSharedMemorySize);
    // uint32_t              maxComputeWorkGroupCount[3];
    //uint32_t              maxComputeWorkGroupInvocations;
    //uint32_t              maxComputeWorkGroupSize[3];
    ADD_ITEM(subPixelPrecisionBits);
    ADD_ITEM(subTexelPrecisionBits);
    ADD_ITEM(mipmapPrecisionBits);
    ADD_ITEM(maxDrawIndexedIndexValue);
    ADD_ITEM(maxDrawIndirectCount);
    ADD_ITEM(maxSamplerLodBias);
    ADD_ITEM(maxSamplerAnisotropy);
    ADD_ITEM(maxViewports);
    //uint32_t              maxViewportDimensions[2];
    //float                 viewportBoundsRange[2];
    ADD_ITEM(viewportSubPixelBits);
    ADD_ITEM(minMemoryMapAlignment);
    ADD_ITEM_SIZE(minTexelBufferOffsetAlignment);
    ADD_ITEM_SIZE(minUniformBufferOffsetAlignment);
    ADD_ITEM_SIZE(minStorageBufferOffsetAlignment);
    ADD_ITEM(minTexelOffset);
    ADD_ITEM(maxTexelOffset);
    ADD_ITEM(minTexelGatherOffset);
    ADD_ITEM(maxTexelGatherOffset);
    ADD_ITEM(minInterpolationOffset);
    ADD_ITEM(maxInterpolationOffset);
    ADD_ITEM(subPixelInterpolationOffsetBits);
    ADD_ITEM(maxFramebufferWidth);
    ADD_ITEM(maxFramebufferHeight);
    ADD_ITEM(maxFramebufferLayers);
    //VkSampleCountFlags    framebufferColorSampleCounts;
    //VkSampleCountFlags    framebufferDepthSampleCounts;
    //VkSampleCountFlags    framebufferStencilSampleCounts;
    //VkSampleCountFlags    framebufferNoAttachmentsSampleCounts;
    ADD_ITEM(maxColorAttachments);
    //VkSampleCountFlags    sampledImageColorSampleCounts;
    //VkSampleCountFlags    sampledImageIntegerSampleCounts;
    //VkSampleCountFlags    sampledImageDepthSampleCounts;
    //VkSampleCountFlags    sampledImageStencilSampleCounts;
    //VkSampleCountFlags    storageImageSampleCounts;
    ADD_ITEM(maxSampleMaskWords);
    ADD_ITEM(timestampComputeAndGraphics);
    ADD_ITEM(timestampPeriod);
    ADD_ITEM(maxClipDistances);
    ADD_ITEM(maxCullDistances);
    ADD_ITEM(maxCombinedClipAndCullDistances);
    ADD_ITEM(discreteQueuePriorities);
    //float                 pointSizeRange[2];
    //float                 lineWidthRange[2];
    ADD_ITEM(pointSizeGranularity);
    ADD_ITEM(lineWidthGranularity);
    ADD_ITEM(strictLines);
    ADD_ITEM(standardSampleLocations);
    ADD_ITEM_SIZE(optimalBufferCopyOffsetAlignment);
    ADD_ITEM_SIZE(optimalBufferCopyRowPitchAlignment);
    ADD_ITEM_SIZE(nonCoherentAtomSize);
#undef ADD_ITEM
#undef ADD_ITEM_SIZE
    return ret;
}
}
