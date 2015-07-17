#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <QDialog>
#include "logmodel.h"
#include "guimodel.h"

namespace Ui {
class LogDialog;
}

class LogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogDialog(LogModel * pLogModel, GuiModel * pGuiModel, QWidget *parent = 0);
    ~LogDialog();

private slots:
    void done(int r);
    void selectLogFile();

    void updatePollTime();
    void updateWriteDuringLog();

private:

    Ui::LogDialog * _pUi;

    LogModel * _pLogModel;
    GuiModel * _pGuiModel;
};

#endif // LOGDIALOG_H
