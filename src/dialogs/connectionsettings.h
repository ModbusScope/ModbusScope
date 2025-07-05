#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

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

    void updateConnectionSettings(quint8 connectionId);
    void updateConnectionState(quint8 connectionId);
    ConnectionForm* connectionSettingsWidget(quint8 connectionId);

    SettingsModel * _pSettingsModel;
};

#endif // CONNECTIONDIALOG_H
