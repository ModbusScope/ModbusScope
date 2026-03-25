#include "adapterconnectionsettings.h"

#include "customwidgets/addabletabwidget.h"
#include "customwidgets/schemaformwidget.h"
#include "models/adapterdata.h"
#include "models/settingsmodel.h"

#include <QJsonArray>
#include <QLabel>
#include <QVBoxLayout>

AdapterConnectionSettings::AdapterConnectionSettings(SettingsModel* pSettingsModel, QWidget* parent)
    : QWidget(parent), _pSettingsModel(pSettingsModel)
{
    auto* layout = new QVBoxLayout(this);
    setLayout(layout);

    // Find first adapter with a populated schema
    const QStringList adapterIds = pSettingsModel->adapterIds();
    for (const auto& id : adapterIds)
    {
        const AdapterData* pAdapter = pSettingsModel->adapterData(id);
        if (!pAdapter->schema().isEmpty())
        {
            _adapterId = id;
            break;
        }
    }

    if (_adapterId.isEmpty())
    {
        layout->addWidget(new QLabel("No adapter schema available.", this));
        layout->addStretch();
        return;
    }

    const AdapterData* pAdapter = pSettingsModel->adapterData(_adapterId);
    QJsonObject schema = pAdapter->schema();
    QJsonObject config = pAdapter->effectiveConfig();

    buildGeneralSection(schema, config, layout);
    buildConnectionsSection(schema, config, layout);
}

void AdapterConnectionSettings::buildGeneralSection(const QJsonObject& schema,
                                                    const QJsonObject& config,
                                                    QVBoxLayout* layout)
{
    QJsonObject generalSchema = schema.value("properties").toObject().value("general").toObject();

    if (generalSchema.value("properties").toObject().isEmpty())
    {
        return;
    }

    layout->addWidget(new QLabel("General", this));
    _pGeneralForm = new SchemaFormWidget(this);
    _pGeneralForm->setSchema(generalSchema, config.value("general").toObject());
    layout->addWidget(_pGeneralForm);
}

void AdapterConnectionSettings::buildConnectionsSection(const QJsonObject& schema,
                                                        const QJsonObject& config,
                                                        QVBoxLayout* layout)
{
    QJsonObject connectionsSchema = schema.value("properties").toObject().value("connections").toObject();
    QString connectionsType = connectionsSchema.value("type").toString();
    QJsonValue connectionsValue = config.value("connections");

    if (connectionsType == "array")
    {
        _connectionItemSchema = connectionsSchema.value("items").toObject();
        _pConnectionTabs = new AddableTabWidget(this);

        QJsonArray connectionsArray = connectionsValue.toArray();
        QList<QWidget*> pages;
        QStringList names;
        for (int i = 0; i < connectionsArray.size(); ++i)
        {
            auto* form = new SchemaFormWidget(_pConnectionTabs);
            form->setSchema(_connectionItemSchema, connectionsArray.at(i).toObject());
            pages.append(form);
            names.append(QString("Connection %1").arg(i + 1));
        }

        if (!pages.isEmpty())
        {
            _pConnectionTabs->setTabs(pages, names);
        }

        connect(_pConnectionTabs, &AddableTabWidget::addTabRequested, this, [this]() {
            auto* form = new SchemaFormWidget(_pConnectionTabs);
            form->setSchema(_connectionItemSchema, QJsonObject());
            _pConnectionTabs->addNewTab(QString("Connection %1").arg(_pConnectionTabs->count() + 1), form);
        });

        layout->addWidget(_pConnectionTabs, 1);
    }
    else
    {
        _pConnectionForm = new SchemaFormWidget(this);
        _pConnectionForm->setSchema(connectionsSchema, connectionsValue.toObject());
        layout->addWidget(_pConnectionForm);
        layout->addStretch();
    }
}

void AdapterConnectionSettings::acceptValues()
{
    if (_adapterId.isEmpty())
    {
        return;
    }

    QJsonObject config = _pSettingsModel->adapterData(_adapterId)->effectiveConfig();

    if (_pGeneralForm)
    {
        config["general"] = _pGeneralForm->values();
    }

    if (_pConnectionTabs)
    {
        QJsonArray connectionsArray;
        for (int i = 0; i < _pConnectionTabs->count(); ++i)
        {
            auto* form = qobject_cast<SchemaFormWidget*>(_pConnectionTabs->tabContent(i));
            if (form)
            {
                connectionsArray.append(form->values());
            }
        }
        config["connections"] = connectionsArray;
    }
    else if (_pConnectionForm)
    {
        config["connections"] = _pConnectionForm->values();
    }

    _pSettingsModel->setAdapterCurrentConfig(_adapterId, config);
}
