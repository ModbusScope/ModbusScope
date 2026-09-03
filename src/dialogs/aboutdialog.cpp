#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "muParserDef.h"
#include "qcustomplot/qcustomplot.h"

#include "models/adapterdata.h"
#include "models/settingsmodel.h"
#include "util/updatenotify.h"
#include "util/util.h"
#include "util/version.h"

#include <QDesktopServices>
#include <QLibraryInfo>
#include <QTextDocument>
#include <QUrl>

AboutDialog::AboutDialog(UpdateNotify* pUpdateNotify, SettingsModel* pSettingsModel, QWidget* parent)
    : QDialog(parent), _pUi(new Ui::AboutDialog)
{
    _pUi->setupUi(this);
    _pUi->textAdapters->document()->setDocumentMargin(12);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(_pUi->btnHomepage, &QPushButton::clicked, this, &AboutDialog::openHomePage);
    connect(_pUi->btnLicense, &QPushButton::clicked, this, &AboutDialog::openLicense);

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
        return;
    }

    QStringList blocks;

    for (const QString& id : adapterIds)
    {
        const AdapterData* pAdapter = pSettingsModel->adapterData(id);

        QString headerTxt = QString("<b>%1</b>").arg((pAdapter->name().isEmpty() ? id : pAdapter->name()).toHtmlEscaped());
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
