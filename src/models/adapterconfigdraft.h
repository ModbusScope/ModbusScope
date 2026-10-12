#ifndef ADAPTERCONFIGDRAFT_H
#define ADAPTERCONFIGDRAFT_H

#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QObject>
#include <QString>

class SettingsModel;

/*!
 * \brief The adapter configs as the settings dialog currently shows them.
 *
 * Each settings page edits one section (one top-level property) of an adapter's config and
 * publishes it here as the user types, without touching SettingsModel. Pages that need to see
 * what other pages are editing, such as the device tabs listing the connections, read the
 * merged result through config() and refresh when configChanged() is emitted.
 *
 * Published sections stay in effect until the draft is destroyed, so a draft belongs to one dialog
 * session. SettingsModel stays the only place where a config is saved. Only sections that a page
 * publishes are visible to other pages; the devices page does not publish its devices yet.
 */
class AdapterConfigDraft : public QObject
{
    Q_OBJECT

public:
    explicit AdapterConfigDraft(SettingsModel* pSettingsModel, QObject* parent = nullptr);

    QJsonObject config(const QString& adapterId) const;
    void setSection(const QString& adapterId, const QString& key, const QJsonValue& value);

signals:
    //! Emitted when the config shown for \a adapterId changed, by a published section or a model update.
    void configChanged(const QString& adapterId);

private slots:
    void onAdapterDataChanged(const QString& adapterId);

private:
    SettingsModel* _pSettingsModel;
    QMap<QString, QJsonObject> _sections;
};

#endif // ADAPTERCONFIGDRAFT_H
