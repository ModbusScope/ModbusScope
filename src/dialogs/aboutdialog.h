#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QByteArray>
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


    static bool requiresOverwriteConfirmation(const AdapterLicenseInfo& existing);

    static QString licensePreviewText(const AdapterLicenseInfo& license, bool replacesValid);

    static QString licenseRejectionText(const AdapterLicenseInfo& license);

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
    static QByteArray hashFile(const QString& path);

    static const int cInspectTimeoutMs = 5000;

    Ui::AboutDialog* _pUi;
    SettingsModel* _pSettingsModel;
    AdapterHub* _pAdapterHub;

    QTimer _inspectTimer;
    bool _inspecting{ false };
    QString _inspectSourcePath;
    QString _inspectDestPath;
    QByteArray _inspectSourceHash;
    bool _inspectReplacesValid{ false };
};

#endif // ABOUTDIALOG_H
