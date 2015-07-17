#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <QDialog>
#include "logmodel.h"

namespace Ui {
class LogDialog;
}

class LogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogDialog(LogModel * pLogModel, QWidget *parent = 0);
    ~LogDialog();

private slots:
    void done(int r);

    void updatePollTime();

private:

    Ui::LogDialog * _pUi;

    LogModel * _pLogModel;
};

#endif // LOGDIALOG_H
