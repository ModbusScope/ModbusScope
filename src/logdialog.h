#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <QDialog>

/* Foward declaration */
class SettingsModel;
class GuiModel;

namespace Ui {
class LogDialog;
}

class LogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogDialog(SettingsModel * pSettingsModel, GuiModel * pGuiModel, QWidget *parent = 0);
    ~LogDialog();

private slots:
    void done(int r);
    void selectLogFile();

    void updatePollTime();
    void updateWriteDuringLog();
    void updateWriteDuringLogPath();
    void updateAbsoluteTime();

private:

    Ui::LogDialog * _pUi;

    SettingsModel * _pSettingsModel;
    GuiModel * _pGuiModel;
};

#endif // LOGDIALOG_H
