#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include "models/connectiontypes.h"
#include <QWidget>

/* Forward declaration */
class SettingsModel;
class ConnectionForm;

namespace Ui {
class ConnectionSettings;
}

class ConnectionSettings : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionSettings(SettingsModel* pSettingsModel, QWidget* parent = nullptr);
    ~ConnectionSettings();

    void acceptValues();

private:
    Ui::ConnectionSettings* _pUi;

    void updateConnectionSettings(ConnectionTypes::connectionId_t connectionId);
    void updateConnectionState(ConnectionTypes::connectionId_t connectionId);
    ConnectionForm* connectionSettingsWidget(ConnectionTypes::connectionId_t connectionId);

    SettingsModel * _pSettingsModel;
};

#endif // CONNECTIONDIALOG_H
