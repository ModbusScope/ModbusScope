#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <QDialog>
#include <QButtonGroup>

/* Forward declaration */
class SettingsModel;

namespace Ui {
class LogDialog;
}

class LogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogDialog(SettingsModel* pSettingsModel, QWidget* parent = nullptr);
    ~LogDialog();

private slots:
    void done(int r);
    void selectLogFile();

    void updatePollTime();
    void updateWriteDuringLog();
    void updateWriteDuringLogFile();

    void timeReferenceUpdated();
    void updateReferenceTime(int id);

private:

    Ui::LogDialog * _pUi;

    SettingsModel * _pSettingsModel;

    QButtonGroup* _pTimeReferenceGroup;
};

#endif // LOGDIALOG_H
