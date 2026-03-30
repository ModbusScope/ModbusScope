#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "dialogs/adapterdevicesettings.h"
#include "dialogs/adaptersettings.h"
#include "dialogs/logsettings.h"
#include "models/adapterdata.h"
#include "models/settingsmodel.h"

#include <QJsonObject>

SettingsDialog::SettingsDialog(SettingsModel* pSettingsModel, QWidget* parent)
    : QDialog(parent), _pUi(new Ui::SettingsDialog), _pSettingsModel(pSettingsModel)
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto font = _pUi->settingsList->font();
    font.setPointSize(font.pointSize() * 1.2);
    _pUi->settingsList->setFont(font);
    _pUi->settingsList->setStyleSheet("QListWidget {padding: 10px;} QListWidget::item { margin: 10px; }");

    // Remove the placeholder page inserted by the UI designer
    while (_pUi->settingsStack->count() > 0)
    {
        _pUi->settingsStack->removeWidget(_pUi->settingsStack->widget(0));
    }

    // Build dynamic pages from schema.properties of all adapters (all keys except "devices")
    const QStringList adapterIds = pSettingsModel->adapterIds();
    for (const auto& id : adapterIds)
    {
        if (pSettingsModel->adapterData(id)->schema().isEmpty())
        {
            continue;
        }

        const QJsonObject schemaProps = pSettingsModel->adapterData(id)->schema().value("properties").toObject();

        for (auto it = schemaProps.constBegin(); it != schemaProps.constEnd(); ++it)
        {
            const QString& key = it.key();
            if (key == "devices")
            {
                continue;
            }

            const QJsonObject propSchema = it.value().toObject();
            if (!AdapterSettings::isRenderableProperty(propSchema))
            {
                continue;
            }

            const QString title = key[0].toUpper() + key.mid(1);
            auto* listItem = new QListWidgetItem(_pUi->settingsList);
            listItem->setText(title);
            listItem->setIcon(QIcon(":/menu_icon/icons/settings.svg"));
            _pUi->settingsList->addItem(listItem);

            auto* page = new AdapterSettings(pSettingsModel, id, key, this);
            _dynamicSettings.append(page);
            _pUi->settingsStack->addWidget(page);
        }
    }

    // Fixed: Devices page
    auto* devItem = new QListWidgetItem(_pUi->settingsList);
    devItem->setText("Devices");
    devItem->setIcon(QIcon(":/menu_icon/icons/circuit-board.svg"));
    _pUi->settingsList->addItem(devItem);

    _pDevSettings = new AdapterDeviceSettings(pSettingsModel, this);
    _pUi->settingsStack->addWidget(_pDevSettings);

    // Fixed: Log page
    auto* logItem = new QListWidgetItem(_pUi->settingsList);
    logItem->setText("Log");
    logItem->setIcon(QIcon(":/menu_icon/icons/clipboard-list.svg"));
    _pUi->settingsList->addItem(logItem);

    _pUi->settingsStack->addWidget(new LogSettings(pSettingsModel, this));

    connect(_pUi->settingsList, &QListWidget::currentRowChanged, this, &SettingsDialog::settingsStackSwitch);
}

SettingsDialog::~SettingsDialog()
{
    delete _pUi;
}

void SettingsDialog::acceptAllValues()
{
    for (auto* page : std::as_const(_dynamicSettings))
    {
        page->acceptValues();
    }
    _pDevSettings->acceptValues();
}

void SettingsDialog::done(int r)
{
    if (r == QDialog::Accepted)
    {
        acceptAllValues();
    }

    QDialog::done(r);
}

void SettingsDialog::settingsStackSwitch(int newRow)
{
    _pUi->settingsStack->setCurrentIndex(newRow);
}
