#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "muParserDef.h"
#include "qcustomplot/qcustomplot.h"

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"
#include "models/adapterdata.h"
#include "models/settingsmodel.h"
#include "util/fileselectionhelper.h"
#include "util/updatenotify.h"
#include "util/util.h"
#include "util/version.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QTextDocument>
#include <QUrl>

AboutDialog::AboutDialog(UpdateNotify* pUpdateNotify, SettingsModel* pSettingsModel, AdapterHub* pAdapterHub,
                         QWidget* parent)
    : QDialog(parent), _pUi(new Ui::AboutDialog), _pSettingsModel(pSettingsModel), _pAdapterHub(pAdapterHub)
{
    _pUi->setupUi(this);
    _pUi->textAdapters->document()->setDocumentMargin(12);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(_pUi->btnHomepage, &QPushButton::clicked, this, &AboutDialog::openHomePage);
    connect(_pUi->btnLicense, &QPushButton::clicked, this, &AboutDialog::openLicense);
    connect(_pUi->btnRequestLicense, &QPushButton::clicked, this, &AboutDialog::openRequestLicense);
    connect(_pUi->btnLoadLicense, &QPushButton::clicked, this, &AboutDialog::loadLicense);

    _inspectTimer.setSingleShot(true);
    _inspectTimer.setInterval(cInspectTimeoutMs);
    connect(&_inspectTimer, &QTimer::timeout, this, &AboutDialog::onInspectLicenseTimeout);
    if (_pAdapterHub != nullptr)
    {
        connect(_pAdapterHub, &AdapterHub::inspectLicenseResult, this, &AboutDialog::onInspectLicenseResult);
        connect(_pAdapterHub, &AdapterHub::inspectLicenseFailed, this, &AboutDialog::onInspectLicenseFailed);
    }

    setVersionInfo();
    setAdapterInfo(pSettingsModel);
    setLibraryVersionInfo();

    showVersionUpdate(pUpdateNotify);
}

AboutDialog::~AboutDialog()
{
    delete _pUi;
}

void AboutDialog::openHomePage(void)
{
    QDesktopServices::openUrl(QUrl("https://modbusscope.com/"));
}

void AboutDialog::openLicense(void)
{
    QDesktopServices::openUrl(QUrl("https://github.com/ModbusScope/ModbusScope/issues"));
}

void AboutDialog::openRequestLicense(void)
{
    QDesktopServices::openUrl(QUrl("https://modbusscope.com/"));
}

//! \brief Lets the user pick which adapter receives the license.
//! \return The adapter id, or an empty string when nothing was selected or no adapter has a license location.
QString AboutDialog::selectLicenseAdapter()
{
    QStringList adapterIds;
    for (const QString& id : _pSettingsModel->adapterIds())
    {
        const AdapterLicenseInfo licenseInfo = _pSettingsModel->adapterData(id)->licenseInfo();
        if (!licenseInfo.path.isEmpty() && QFileInfo(licenseInfo.path).isAbsolute())
        {
            adapterIds << id;
        }
    }

    if (adapterIds.isEmpty())
    {
        QMessageBox::warning(this, tr("Load License"),
                             tr("None of the configured adapters have reported a license file location."));
        return QString();
    }

    QString adapterId = adapterIds.first();
    if (adapterIds.size() > 1)
    {
        /* Adapter names come from the adapter itself (or fall back to its id) and are not
         * guaranteed unique, so count occurrences first and only disambiguate names that
         * actually collide - otherwise two same-named adapters would map to the same list
         * entry and one would be unreachable. */
        QMap<QString, int> nameCounts;
        for (const QString& id : adapterIds)
        {
            const AdapterData* pAdapter = _pSettingsModel->adapterData(id);
            const QString baseName = pAdapter->name().isEmpty() ? id : pAdapter->name();
            nameCounts[baseName]++;
        }

        QStringList displayNames;
        QMap<QString, QString> nameToId;
        for (const QString& id : adapterIds)
        {
            const AdapterData* pAdapter = _pSettingsModel->adapterData(id);
            const QString baseName = pAdapter->name().isEmpty() ? id : pAdapter->name();
            const QString displayName =
              nameCounts.value(baseName) > 1 ? QString("%1 (%2)").arg(baseName, id) : baseName;
            displayNames << displayName;
            nameToId.insert(displayName, id);
        }

        bool bOk = false;
        const QString chosenName =
          QInputDialog::getItem(this, tr("Load License"), tr("Select adapter:"), displayNames, 0, false, &bOk);
        if (!bOk)
        {
            return QString();
        }

        adapterId = nameToId.value(chosenName);
    }

    return adapterId;
}

void AboutDialog::loadLicense(void)
{
    const QString adapterId = selectLicenseAdapter();
    if (adapterId.isEmpty())
    {
        return;
    }

    QFileDialog dialog(this);
    FileSelectionHelper::configureFileDialog(&dialog, FileSelectionHelper::DIALOG_TYPE_OPEN,
                                             FileSelectionHelper::FILE_TYPE_LIC);
    const QString sourcePath = FileSelectionHelper::showDialog(&dialog);
    if (sourcePath.isEmpty())
    {
        return;
    }

    startLicenseInspection(adapterId, sourcePath);
}

//! \brief Asks the adapter to verify the chosen license file; nothing is installed until the user confirms.
void AboutDialog::startLicenseInspection(const QString& adapterId, const QString& sourcePath)
{
    const AdapterManager* pManager = (_pAdapterHub != nullptr) ? _pAdapterHub->adapterManager(adapterId) : nullptr;
    if (pManager == nullptr || !(pManager->isAdapterReady() || pManager->isAdapterActive()))
    {
        QMessageBox::warning(this, tr("Load License"), tr("Adapter is not running, cannot verify the license."));
        return;
    }

    const AdapterLicenseInfo existing = _pSettingsModel->adapterData(adapterId)->licenseInfo();
    _inspectDestPath = existing.path;
    _inspectReplacesValid = requiresOverwriteConfirmation(existing);
    _inspectSourcePath = sourcePath;
    _inspecting = true;
    _pUi->btnLoadLicense->setEnabled(false);
    _inspectTimer.start();

    _pAdapterHub->inspectLicense(adapterId, QFileInfo(sourcePath).absoluteFilePath());
}

//! \brief Ends a pending inspection: stops the timeout and re-enables the load button.
void AboutDialog::finishLicenseInspection()
{
    _inspecting = false;
    _inspectTimer.stop();
    _pUi->btnLoadLicense->setEnabled(true);
}

void AboutDialog::onInspectLicenseResult(const QJsonObject& result)
{
    if (!_inspecting)
    {
        return;
    }
    finishLicenseInspection();

    const AdapterLicenseInfo license = AdapterLicenseInfo::fromJson(result);
    if (license.state != AdapterLicenseInfo::State::Valid)
    {
        QMessageBox box(QMessageBox::Warning, tr("Load License"), licenseRejectionText(license), QMessageBox::Ok, this);
        box.setTextFormat(Qt::RichText);
        box.exec();
        return;
    }

    confirmAndInstallLicense(license);
}

void AboutDialog::onInspectLicenseFailed(const QString& message)
{
    Q_UNUSED(message);
    if (!_inspecting)
    {
        return;
    }
    finishLicenseInspection();
    showInspectionUnsupported();
}

void AboutDialog::onInspectLicenseTimeout()
{
    if (!_inspecting)
    {
        return;
    }
    finishLicenseInspection();
    showInspectionUnsupported();
}

void AboutDialog::showInspectionUnsupported()
{
    QMessageBox::warning(this, tr("Load License"),
                         tr("This adapter cannot verify licenses. Please update the adapter."));
}

//! \brief Shows the verified license details and installs the file only if the user confirms.
void AboutDialog::confirmAndInstallLicense(const AdapterLicenseInfo& license)
{
    QMessageBox box(QMessageBox::Question, tr("Load License"), licensePreviewText(license, _inspectReplacesValid),
                    QMessageBox::Yes | QMessageBox::No, this);
    box.setTextFormat(Qt::RichText);
    box.setDefaultButton(QMessageBox::No);
    if (box.exec() != QMessageBox::Yes)
    {
        return;
    }

    const QString error = installLicenseFile(_inspectSourcePath, _inspectDestPath);
    if (!error.isEmpty())
    {
        QMessageBox::warning(this, tr("Load License"), tr("Failed to install license file: %1").arg(error));
        return;
    }

    QMessageBox::information(this, tr("Load License"),
                             tr("License installed. It will take effect the next time the adapter is "
                                "initialized (e.g. after restarting ModbusScope)."));
}

void AboutDialog::showVersionUpdate(UpdateNotify* updateNotify)
{
    if (updateNotify->versionState() == UpdateNotify::VERSION_LATEST)
    {
        _pUi->lblUpdate->setText("No update available");
    }
    else
    {
        QString updateTxt;

        updateTxt.append(QString("Update available: <a href=\'%1\'>v%2</a>")
                           .arg(updateNotify->link().toString(), updateNotify->version()));

        updateTxt.append("<br/>");
        _pUi->lblUpdate->setText(updateTxt);
    }

    _pUi->lblUpdate->setVisible(true);
}

void AboutDialog::setVersionInfo()
{
    QString debugTxt;
    QString betaTxt;
#ifdef DEBUG
    debugTxt = QString(tr("(git: %1:%2)")).arg(GIT_BRANCH, GIT_COMMIT_HASH);
    betaTxt = QString(tr(" beta"));
#endif

    QString arch;
    if (sizeof(void*) == 4)
    {
        arch = "32 bit";
    }
    else if (sizeof(void*) == 8)
    {
        arch = "64 bit";
    }
    else
    {
        arch = "unknown";
    }

    _pUi->lblDebug->setText(debugTxt);
    _pUi->lblVersion->setText(QString(tr("v%1%2 (%3)")).arg(Util::currentVersion(), betaTxt, arch));
}

void AboutDialog::setAdapterInfo(SettingsModel* pSettingsModel)
{
    const QStringList adapterIds = pSettingsModel->adapterIds();

    if (adapterIds.isEmpty())
    {
        _pUi->textAdapters->setVisible(false);
        _pUi->btnLoadLicense->setVisible(false);
        return;
    }

    QStringList blocks;

    for (const QString& id : adapterIds)
    {
        const AdapterData* pAdapter = pSettingsModel->adapterData(id);

        QString headerTxt =
          QString("<b>%1</b>").arg((pAdapter->name().isEmpty() ? id : pAdapter->name()).toHtmlEscaped());
        if (!pAdapter->version().isEmpty())
        {
            headerTxt += QString("<br/>v%1").arg(pAdapter->version().toHtmlEscaped());
        }

        blocks << QString("<p style=\"margin-top:8px;\">%1<br/>%2</p>")
                    .arg(headerTxt, licenseInfoHtml(pAdapter->licenseInfo()));
    }

    _pUi->textAdapters->setHtml(blocks.join("<hr/>"));
    _pUi->textAdapters->setVisible(true);
}

//! \brief Build an HTML fragment describing an adapter's license state, with all
//! adapter-supplied fields HTML-escaped since they originate from the adapter subprocess.
QString AboutDialog::licenseInfoHtml(const AdapterLicenseInfo& license)
{
    switch (license.state)
    {
    case AdapterLicenseInfo::State::Valid:
    {
        QString txt =
          tr("Licensed to %1, ID %2").arg(license.customer.toHtmlEscaped(), license.licenseId.toHtmlEscaped());
        if (!license.email.isEmpty())
        {
            txt += QString(" &lt;%1&gt;").arg(license.email.toHtmlEscaped());
        }
        if (!license.expires.isEmpty())
        {
            txt += tr(", expires %1").arg(license.expires.toHtmlEscaped());
        }
        return txt;
    }
    case AdapterLicenseInfo::State::Invalid:
        return tr("License invalid: %1").arg(license.reason.toHtmlEscaped());
    case AdapterLicenseInfo::State::NotFound:
        return tr("No license found (searched %1)").arg(license.path.toHtmlEscaped());
    case AdapterLicenseInfo::State::Unknown:
        return tr("No license information reported");
    }

    return tr("No license information reported");
}

//! \brief Builds the rich-text confirmation shown before a verified license is installed.
QString AboutDialog::licensePreviewText(const AdapterLicenseInfo& license, bool replacesValid)
{
    QStringList lines;
    lines << tr("This license was issued to:");
    lines << tr("Customer: %1").arg(license.customer.toHtmlEscaped());
    if (!license.email.isEmpty())
    {
        lines << tr("Email: %1").arg(license.email.toHtmlEscaped());
    }
    lines << tr("License ID: %1").arg(license.licenseId.toHtmlEscaped());
    if (!license.expires.isEmpty())
    {
        lines << tr("Expires: %1").arg(license.expires.toHtmlEscaped());
    }

    QString text = lines.join("<br/>");
    text += "<br/><br/>" + tr("Only load this license if it was issued to you or your organisation.");
    if (replacesValid)
    {
        text += "<br/><br/>" + tr("A valid license is already installed for this adapter and will be replaced.");
    }
    text += "<br/><br/>" + tr("Load this license?");
    return text;
}

//! \brief Builds the rich-text explanation for a license that will not be installed.
QString AboutDialog::licenseRejectionText(const AdapterLicenseInfo& license)
{
    if (license.state == AdapterLicenseInfo::State::NotFound)
    {
        return tr("License file not found.");
    }

    return tr("The license is not valid and was not installed: %1").arg(license.reason.toHtmlEscaped());
}

//! \brief Returns true only when a valid license is already installed, so a fresh, invalid,
//! or missing license can be replaced without prompting.
bool AboutDialog::requiresOverwriteConfirmation(const AdapterLicenseInfo& existing)
{
    return existing.state == AdapterLicenseInfo::State::Valid;
}

//! \brief Copies a license file to its installed location, creating the destination
//! directory and replacing any existing file at destPath.
//!
//! Copies to a temporary file first and only then swaps it into place, so a failed or
//! aborted copy never leaves destPath deleted with nothing to replace it - important since
//! sourcePath and destPath can be the same file (the user re-picking the already-installed
//! license from its own directory).
QString AboutDialog::installLicenseFile(const QString& sourcePath, const QString& destPath)
{
    const QDir destDir = QFileInfo(destPath).dir();
    if (!destDir.exists() && !QDir().mkpath(destDir.absolutePath()))
    {
        return tr("could not create directory %1").arg(destDir.absolutePath());
    }

    const QString tempPath = destPath + ".tmp";
    QFile::remove(tempPath);
    if (!QFile::copy(sourcePath, tempPath))
    {
        return tr("could not copy file to %1").arg(destPath);
    }

    if (QFile::exists(destPath) && !QFile::remove(destPath))
    {
        QFile::remove(tempPath);
        return tr("could not replace existing file at %1").arg(destPath);
    }

    if (!QFile::rename(tempPath, destPath))
    {
        return tr("could not finalize install at %1").arg(destPath);
    }

    return QString();
}

void AboutDialog::setLibraryVersionInfo()
{
    QString qtVersion(QLibraryInfo::version().toString());
    QString plotVersion(QCUSTOMPLOT_VERSION_STR);
    QString parserVersion = QString::fromStdWString(mu::ParserVersion);

    QString aboutText = _pUi->textAbout->toHtml();

    aboutText.replace("QT_VERSION", qtVersion);
    aboutText.replace("QCUSTOM_PLOT_VERSION", plotVersion);
    aboutText.replace("MUPARSER_VERSION", parserVersion);

    _pUi->textAbout->setHtml(aboutText);
}
