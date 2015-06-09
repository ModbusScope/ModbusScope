#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>
#include "connectionmodel.h"

namespace Ui {
class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(ConnectionModel * pConnectionModel, QWidget *parent = 0);
    ~ConnectionDialog();

private slots:
    void done(int r);

    void updateIp();
    void updatePort();
    void updatePollTime();
    void updateSlaveId();
    void updateTimeout();

private:
    Ui::ConnectionDialog * _pUi;

    ConnectionModel * _pConnectionModel;
};

#endif // CONNECTIONDIALOG_H
