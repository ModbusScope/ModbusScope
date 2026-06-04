#ifndef ADAPTERHUB_H
#define ADAPTERHUB_H

#include "util/result.h"

#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>

inline constexpr char cModbusAdapterId[] = "modbus";

class AdapterManager;
class SettingsModel;

/*!
 * \brief Owns and coordinates all discovered adapter managers.
 *
 * On initAdapter(), AdapterHub uses AdapterDiscovery to locate adapter binaries,
 * creates one AdapterManager per binary, and gates the adapterReady() and
 * sessionStarted() signals until all managers have reached the required state.
 *
 * Virtual methods allow a mock subclass to be injected in unit tests via the
 * protected constructor.
 */
class AdapterHub : public QObject
{
    Q_OBJECT
public:
    explicit AdapterHub(SettingsModel* pSettingsModel, QObject* parent = nullptr);

    virtual void initAdapter();
    virtual void startSession(const QString& adapterId, const QStringList& expressions);
    virtual void stopSession();
    virtual void requestReadData();

    virtual AdapterManager* adapterManager(const QString& id) const;
    virtual bool isAdapterReady() const;
    virtual bool isAdapterIdle() const;

signals:
    void sessionStarted();
    void sessionStopped();
    void adapterReady();
    void sessionError(QString message);
    void readDataResult(const QString& adapterId, ResultDoubleList results);
    void buildExpressionResult(QString expression);
    void expressionHelpResult(QString helpText);
    void describeDataPointResult(QJsonObject result);

protected:
    explicit AdapterHub(QObject* parent = nullptr);

private slots:
    void onManagerAdapterReady(const QString& id);
    void onManagerSessionStarted(const QString& id);

private:
    void connectManager(AdapterManager* mgr, const QString& id);

    SettingsModel* _pSettingsModel;
    QMap<QString, AdapterManager*> _adapterManagers;
    QSet<QString> _pendingReadyAdapters;
    QSet<QString> _pendingStartAdapters;
};

#endif // ADAPTERHUB_H
