#include "adapterconfigdraft.h"

#include "models/adapterdata.h"
#include "models/settingsmodel.h"

AdapterConfigDraft::AdapterConfigDraft(SettingsModel* pSettingsModel, QObject* parent)
    : QObject(parent), _pSettingsModel(pSettingsModel)
{
    connect(_pSettingsModel, &SettingsModel::adapterDataChanged, this, &AdapterConfigDraft::onAdapterDataChanged);
}

/*!
 * \brief Returns the saved config of \a adapterId with the published sections applied.
 * \param adapterId  The adapter identifier string.
 * \return The merged config.
 */
QJsonObject AdapterConfigDraft::config(const QString& adapterId) const
{
    QJsonObject result = _pSettingsModel->adapterData(adapterId)->effectiveConfig();
    const QJsonObject sections = _sections.value(adapterId);
    for (auto it = sections.constBegin(); it != sections.constEnd(); ++it)
    {
        result.insert(it.key(), it.value());
    }
    return result;
}

/*!
 * \brief Publishes the section \a key of \a adapterId, replacing an earlier one.
 * \param adapterId  The adapter identifier string.
 * \param key        Top-level config property edited by the publishing page.
 * \param value      The page's current value for the property.
 */
void AdapterConfigDraft::setSection(const QString& adapterId, const QString& key, const QJsonValue& value)
{
    QJsonObject sections = _sections.value(adapterId);
    if (sections.contains(key) && sections.value(key) == value)
    {
        return;
    }

    sections.insert(key, value);
    _sections.insert(adapterId, sections);
    emit configChanged(adapterId);
}

//! A saved config change alters the merged config, so listeners must refresh.
void AdapterConfigDraft::onAdapterDataChanged(const QString& adapterId)
{
    emit configChanged(adapterId);
}
