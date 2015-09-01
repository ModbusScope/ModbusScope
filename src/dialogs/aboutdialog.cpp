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

    QString lnkAuthor("<a href='https://github.com/jgeudens'>jgeudens</a>");
    QString lnkGpl("<a href='http://www.gnu.org/licenses/gpl.html#content'>GPL</a>");
    QString lnkGitHub("<a href='https://github.com/jgeudens/ModbusScope'>GitHub</a>");

    QString lnkQt("<a href='http://qt-project.org/'>Qt</a>");
    QString lnkLibModbus("<a href='http://libmodbus.org/'>libmodbus</a>");
    QString lnkQCustomPlot("<a href='http://www.qcustomplot.com/'>QCustomPlot</a>");

    QString aboutTxt = tr(
                        "ModbusScope is created and maintained by %1. This software is released under the %2 license. "
                        "The source is freely available at %3.<br><br>"
                        "ModbusScope uses following libraries:"
                        "<ul><li>%5</li>"
                        "<li>%6</li>"
                        "<li>%7</li></ul>").arg(lnkAuthor, lnkGpl, lnkGitHub, lnkQt, lnkLibModbus, lnkQCustomPlot);

    _pUi->textAbout->setOpenExternalLinks(true);
    _pUi->textAbout->setText(aboutTxt);

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
