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
#include <QUrl>

AboutDialog::AboutDialog(UpdateNotify* pUpdateNotify, SettingsModel* pSettingsModel, QWidget* parent)
    : QDialog(parent), _pUi(new Ui::AboutDialog)
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(_pUi->btnHomepage, &QPushButton::clicked, this, &AboutDialog::openHomePage);
    connect(_pUi->btnLicense, &QPushButton::clicked, this, &AboutDialog::openLicense);

    setVersionInfo();
    setAdapterVersionInfo(pSettingsModel);
    setLibraryVersionInfo();

    showVersionUpdate(pUpdateNotify);
}

AboutDialog::~AboutDialog()
{
    delete _pUi;
}

void AboutDialog::openHomePage(void)
{
    QDesktopServices::openUrl(QUrl("https://github.com/ModbusScope/ModbusScope"));
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

void AboutDialog::setAdapterVersionInfo(SettingsModel* pSettingsModel)
{
    QString versionTxt;

    for (const QString& id : pSettingsModel->adapterIds())
    {
        const QString version = pSettingsModel->adapterData(id)->version();
        if (!version.isEmpty())
        {
            versionTxt = QString(tr("Adapter: v%1")).arg(version);
            break;
        }
    }

    _pUi->lblAdapterVersion->setVisible(!versionTxt.isEmpty());
    _pUi->lblAdapterVersion->setText(versionTxt);
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
