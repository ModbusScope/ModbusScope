#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QDesktopServices>
#include <QUrl>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::AboutDialog)
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(_pUi->btnHomepage, SIGNAL(clicked()), this, SLOT(OpenHomePage()));
    connect(_pUi->btnLicense, SIGNAL(clicked()), this, SLOT(OpenLicense()));

    _pUi->lblVersion->setText(QString(tr("v%1")).arg(APP_VERSION));

#ifdef DEBUG
    _pUi->lblDebug->setText(QString(tr("(git: %1:%2)")).arg(GIT_BRANCH).arg(GIT_HASH));
#else
     _pUi->lblDebug->setText("");
#endif

    _pUi->textAbout->setOpenExternalLinks(true);

}

AboutDialog::~AboutDialog()
{
    delete _pUi;
}

void AboutDialog::OpenHomePage(void)
{
    QDesktopServices::openUrl(QUrl("http://jgeudens.github.io"));
}

void AboutDialog::OpenLicense(void)
{
    QDesktopServices::openUrl(QUrl("http://www.gnu.org/licenses/gpl-3.0-standalone.html"));
}
