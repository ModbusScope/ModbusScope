#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

// Forward declaration
class UpdateNotify;
class SettingsModel;

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(UpdateNotify* pUpdateNotify, SettingsModel* pSettingsModel, QWidget* parent = nullptr);
    ~AboutDialog();

private slots:
    void openHomePage(void);
    void openLicense(void);

private:
    void showVersionUpdate(UpdateNotify* updateNotify);
    void setVersionInfo();
    void setAdapterVersionInfo(SettingsModel* pSettingsModel);
    void setLibraryVersionInfo();

    Ui::AboutDialog* _pUi;
};

#endif // ABOUTDIALOG_H
