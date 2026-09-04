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

    /*!
     * \brief Returns true when the given license state should require the user to confirm
     * before overwriting an existing license file with a new one.
     */
    static bool requiresOverwriteConfirmation(const AdapterLicenseInfo& existing);

    /*!
     * \brief Copies a license file to its installed location.
     *
     * Creates the destination directory if needed, replacing any existing file at destPath.
     * \return An empty string on success, or a human-readable error message on failure.
     */
    static QString installLicenseFile(const QString& sourcePath, const QString& destPath);

private slots:
    void openHomePage(void);
    void openLicense(void);
    void loadLicense(void);

private:
    void showVersionUpdate(UpdateNotify* updateNotify);
    void setVersionInfo();
    void setAdapterInfo(SettingsModel* pSettingsModel);
    void setLibraryVersionInfo();
    static QString licenseInfoHtml(const AdapterLicenseInfo& license);

    Ui::AboutDialog* _pUi;
    SettingsModel* _pSettingsModel;
};

#endif // ABOUTDIALOG_H
