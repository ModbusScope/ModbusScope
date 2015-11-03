#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QDesktopServices>
#include <QUrl>

#include "util.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::AboutDialog)
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(_pUi->btnHomepage, SIGNAL(clicked()), this, SLOT(openHomePage()));
    connect(_pUi->btnLicense, SIGNAL(clicked()), this, SLOT(openLicense()));

    _pUi->lblVersion->setText(QString(tr("v%1")).arg(Util::currentVersion()));

#ifdef DEBUG
    _pUi->lblDebug->setText(QString(tr("(git: %1:%2)")).arg(GIT_BRANCH).arg(GIT_HASH));
#else
     _pUi->lblDebug->setText("");
#endif

    _pUi->textAbout->setOpenExternalLinks(true);

    connect(&_updateNotify, SIGNAL(updateCheckResult(UpdateNotify::UpdateState,bool)), this, SLOT(showVersionUpdate(UpdateNotify::UpdateState, bool)));
    _updateNotify.checkForUpdate();
}

AboutDialog::~AboutDialog()
{
    delete _pUi;
}

void AboutDialog::openHomePage(void)
{
    QDesktopServices::openUrl(QUrl("http://jgeudens.github.io"));
}

void AboutDialog::openLicense(void)
{
    QDesktopServices::openUrl(QUrl("http://www.gnu.org/licenses/gpl-3.0-standalone.html"));
}

void AboutDialog::showVersionUpdate(UpdateNotify::UpdateState state, bool bDataLevelUpdate)
{
    if (state == UpdateNotify::LATEST)
    {
        _pUi->lblUpdate->setText("No update available");
    }
    else
    {
        QString updateTxt;

        updateTxt.append(QString("Update available: <a href=\'%1\'>v%2</a>").arg(_updateNotify.link()).arg(_updateNotify.version()));

        if (bDataLevelUpdate)
        {
            updateTxt.append("<br/><br/>Warning: Datalevel has changed. New version is not compatible with existing project files.");
        }

        updateTxt.append("<br/>");
        _pUi->lblUpdate->setText(updateTxt);
    }
}
