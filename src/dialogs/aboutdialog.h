#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

// Forward declaration
class UpdateNotify;
class SettingsModel;
struct AdapterLicenseInfo;

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
    void setAdapterInfo(SettingsModel* pSettingsModel);
    void setLibraryVersionInfo();
    static QString licenseInfoHtml(const AdapterLicenseInfo& license);

    Ui::AboutDialog* _pUi;
};

#endif // ABOUTDIALOG_H
