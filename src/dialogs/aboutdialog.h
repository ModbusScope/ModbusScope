#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include <QTimer>

// Forward declaration
class AdapterHub;
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
    explicit AboutDialog(UpdateNotify* pUpdateNotify, SettingsModel* pSettingsModel, AdapterHub* pAdapterHub,
                         QWidget* parent = nullptr);
    ~AboutDialog();

    /*!
     * \brief Returns true when the given license state should require the user to confirm
     * before overwriting an existing license file with a new one.
     */
    static bool requiresOverwriteConfirmation(const AdapterLicenseInfo& existing);

    /*!
     * \brief Builds the confirmation text shown before a verified license is installed.
     *
     * All adapter-supplied fields are HTML-escaped; the result is meant for Qt::RichText.
     * \param license The valid license reported by the adapter.
     * \param replacesValid True when a valid license is already installed and would be overwritten.
     */
    static QString licensePreviewText(const AdapterLicenseInfo& license, bool replacesValid);

    /*!
     * \brief Builds the text explaining why a license that is not valid will not be installed.
     *
     * The result is HTML-escaped and meant for Qt::RichText.
     */
    static QString licenseRejectionText(const AdapterLicenseInfo& license);

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
    void openRequestLicense(void);
    void loadLicense(void);
    void onInspectLicenseResult(const QJsonObject& result);
    void onInspectLicenseFailed(const QString& message);
    void onInspectLicenseTimeout();

private:
    void showVersionUpdate(UpdateNotify* updateNotify);
    void setVersionInfo();
    void setAdapterInfo(SettingsModel* pSettingsModel);
    void setLibraryVersionInfo();
    static QString licenseInfoHtml(const AdapterLicenseInfo& license);

    QString selectLicenseAdapter();
    void startLicenseInspection(const QString& adapterId, const QString& sourcePath);
    void finishLicenseInspection();
    void confirmAndInstallLicense(const AdapterLicenseInfo& license);
    void showInspectionUnsupported();

    static const int cInspectTimeoutMs = 5000;

    Ui::AboutDialog* _pUi;
    SettingsModel* _pSettingsModel;
    AdapterHub* _pAdapterHub;

    QTimer _inspectTimer;
    bool _inspecting{ false };
    QString _inspectSourcePath;
    QString _inspectDestPath;
    bool _inspectReplacesValid{ false };
};

#endif // ABOUTDIALOG_H
