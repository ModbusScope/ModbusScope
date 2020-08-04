#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "qcustomplot.h"

#include <QDesktopServices>
#include <QUrl>
#include <QLibraryInfo>

#include "util.h"
#include "version.h"
#include "versiondownloader.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::AboutDialog),
    _updateNotify(new VersionDownloader(), Util::currentVersion())
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(_pUi->btnHomepage, &QPushButton::clicked, this, &AboutDialog::openHomePage);
    connect(_pUi->btnLicense, &QPushButton::clicked, this, &AboutDialog::openLicense);

    setVersionInfo();
    setLibraryVersionInfo();

    connect(&_updateNotify, &UpdateNotify::updateCheckResult, this, &AboutDialog::showVersionUpdate);
    _updateNotify.checkForUpdate();

    _pUi->lblUpdate->setVisible(false);
}

AboutDialog::~AboutDialog()
{
    delete _pUi;
}

void AboutDialog::openHomePage(void)
{
    QDesktopServices::openUrl(QUrl("https://github.com/jgeudens/ModbusScope/releases"));
}

void AboutDialog::openLicense(void)
{
    QDesktopServices::openUrl(QUrl("http://www.gnu.org/licenses/gpl-3.0-standalone.html"));
}

void AboutDialog::showVersionUpdate(UpdateNotify::UpdateState state)
{
    if (state == UpdateNotify::VERSION_LATEST)
    {
        _pUi->lblUpdate->setText("No update available");
    }
    else
    {
        QString updateTxt;

        updateTxt.append(QString("Update available: <a href=\'%1\'>v%2</a>").arg(_updateNotify.link().toString()).arg(_updateNotify.version()));

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
    debugTxt = QString(tr("(git: %1:%2)")).arg(GIT_BRANCH).arg(GIT_COMMIT_HASH);
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
    _pUi->lblVersion->setText(QString(tr("v%1%2 (%3)")).arg(Util::currentVersion()).arg(betaTxt).arg(arch));
}

void AboutDialog::setLibraryVersionInfo()
{
    QString qtVersion(QLibraryInfo::version().toString());
    QString plotVersion(QCUSTOMPLOT_VERSION_STR);

    QString aboutText = _pUi->textAbout->toHtml();

    aboutText.replace("QT_VERSION", qtVersion);
    aboutText.replace("QCUSTOM_PLOT_VERSION", plotVersion);

    _pUi->textAbout->setHtml(aboutText);
}

