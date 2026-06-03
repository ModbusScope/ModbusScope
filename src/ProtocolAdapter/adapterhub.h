#ifndef ADAPTERHUB_H
#define ADAPTERHUB_H

#include "util/result.h"

#include <QJsonObject>
#include <QObject>
#include <QStringList>

class AdapterManager;
class SettingsModel;

/*!
 * \brief Owns and coordinates the adapter manager(s) for the application.
 *
 * In the current single-adapter implementation, AdapterHub creates and owns one
 * AdapterManager and forwards its lifecycle signals. Future phases will extend
 * this class to discover and manage multiple adapter binaries.
 *
 * Virtual methods allow a mock subclass to be injected in unit tests.
 */
class AdapterHub : public QObject
{
    Q_OBJECT
public:
    explicit AdapterHub(SettingsModel* pSettingsModel, QObject* parent = nullptr);

    virtual void initAdapter();
    virtual void startSession(const QStringList& expressions);
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
    void readDataResult(ResultDoubleList results);
    void buildExpressionResult(QString expression);
    void expressionHelpResult(QString helpText);
    void describeDataPointResult(QJsonObject result);

protected:
    explicit AdapterHub(QObject* parent = nullptr);

private:
    AdapterManager* _pAdapterManager;
};

#endif // ADAPTERHUB_H
