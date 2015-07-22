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
    explicit ConnectionDialog(SettingsModel * pSettingsModel, QWidget *parent = 0);
    ~ConnectionDialog();

private slots:
    void done(int r);

    void updateIp();
    void updatePort();
    void updateSlaveId();
    void updateTimeout();

private:
    Ui::ConnectionDialog * _pUi;

    SettingsModel * _pSettingsModel;
};

#endif // CONNECTIONDIALOG_H
