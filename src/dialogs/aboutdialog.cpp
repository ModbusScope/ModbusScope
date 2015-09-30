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

#if 0
    if (versionState == 0)
    {
        if (_bManual)
        {
            // Current is latest (or newer), no update needed
            QMessageBox::information(_pWidget, "Update check", "You already have the latest version");
        }
    }
    else
    {
        //if (_versionDownloader.severity() >= /* Settings allowed update*/)
        {

            /* Create text */
            QString updateText = QString("Current version: <b>v%1</b>").arg(Util::currentVersion());
            updateText.append(QString("<br/>New version found: <b>v%1</b>").arg(_versionDownloader.version()));

            if (_versionDownloader.dataLevel() != Util::currentDataLevel())
            {
                /* Add datalevel info to dialog */
                updateText.append("<br/><br/>Note that the project file structure has changed and is not compatible with previous versions."
                                  "You will not be able to load existing project files");
            }

            updateText.append("<br/><br/>Do you want to download the update now?<br/>");

            /* Show dialog */
            QMessageBox msgBox;
            //msgBox.setParent(_pWidget);
            QAbstractButton * skipReleaseButton = msgBox.addButton(trUtf8("Skip this release"), QMessageBox::RejectRole);
            msgBox.addButton(trUtf8("No"), QMessageBox::NoRole);
            QAbstractButton * yesButton = msgBox.addButton(trUtf8("Yes"), QMessageBox::YesRole);
            msgBox.setText(updateText);
            msgBox.setDefaultButton((QPushButton *)yesButton);
            msgBox.exec();

            if (msgBox.clickedButton() == yesButton)
            {
                QDesktopServices::openUrl(QUrl(_versionDownloader.link()));
            }
            else if (msgBox.clickedButton() == skipReleaseButton)
            {
                // Save skip to QSetting
            }
            else
            {
                // No
            }
        }
    }


    typedef enum
    {
        LATEST = 0,             /* Latest version */
        REV_UPDATE,             /* Revision update */
        MINOR_MAJOR_UPDATE,     /* Minor/major update */
    } UpdateState;

#endif
}
