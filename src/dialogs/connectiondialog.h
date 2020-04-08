#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>


/* Forward declaration */
class SettingsModel;

namespace Ui {
class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(SettingsModel * pSettingsModel, QWidget *parent = nullptr);
    ~ConnectionDialog();

private slots:
    void done(int r);

    void secondConnectionStateChanged(int state);

    void updateIp(quint8 connectionId);
    void updatePort(quint8 connectionId);
    void updateSlaveId(quint8 connectionId);
    void updateTimeout(quint8 connectionId);
    void updateConsecutiveMax(quint8 connectionId);
    void updateConnectionState(quint8 connectionId);
    void updateInt32LittleEndian(quint8 connectionId);
    void updatePersistentConnection(quint8 connectionId);

private:
    Ui::ConnectionDialog * _pUi;

    SettingsModel * _pSettingsModel;
};

#endif // CONNECTIONDIALOG_H
