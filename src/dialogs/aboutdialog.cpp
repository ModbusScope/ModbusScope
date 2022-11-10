#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "qcustomplot.h"
#include "muParserDef.h"

#include <QDesktopServices>
#include <QUrl>
#include <QLibraryInfo>

#include "util.h"
#include "version.h"
#include "versiondownloader.h"

AboutDialog::AboutDialog(UpdateNotify *pUpdateNotify, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::AboutDialog)
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(_pUi->btnHomepage, &QPushButton::clicked, this, &AboutDialog::openHomePage);
    connect(_pUi->btnLicense, &QPushButton::clicked, this, &AboutDialog::openLicense);

    setVersionInfo();
    setLibraryVersionInfo();

    showVersionUpdate(pUpdateNotify);
}

AboutDialog::~AboutDialog()
{
    delete _pUi;
}

void AboutDialog::openHomePage(void)
{
    QDesktopServices::openUrl(QUrl("https://github.com/jgeudens/ModbusScope"));
}

void AboutDialog::openLicense(void)
{
    QDesktopServices::openUrl(QUrl("https://github.com/jgeudens/ModbusScope/issues"));
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

        updateTxt.append(QString("Update available: <a href=\'%1\'>v%2</a>").arg(updateNotify->link().toString(), updateNotify->version()));

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

