#ifndef LOGSETTINGS_H
#define LOGSETTINGS_H

#include <QWidget>
#include <QButtonGroup>

/* Forward declaration */
class SettingsModel;

namespace Ui {
class LogSettings;
}

class LogSettings : public QWidget
{
    Q_OBJECT

public:
    explicit LogSettings(SettingsModel* pSettingsModel, QWidget* parent = nullptr);
    ~LogSettings();

private slots:
    void selectLogFile();
    void handlePollTime(int pollTime);
    void handleLogFile();
    void updateWriteDuringLog();

    void updateReferenceTime(int id);

private:
    Ui::LogSettings* _pUi;

    SettingsModel * _pSettingsModel;

    QButtonGroup* _pTimeReferenceGroup;
};

#endif // LOGSETTINGS_H
