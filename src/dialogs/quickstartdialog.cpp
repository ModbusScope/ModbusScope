#include "quickstartdialog.h"
#include "ui_quickstartdialog.h"

#include <QPushButton>

QuickStartDialog::QuickStartDialog(QWidget* parent) : QDialog(parent), _pUi(new Ui::QuickStartDialog)
{
    _pUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    // Qt uic 6.8.3 bug: generates invalid setContentsMargins(20) (single-int overload)
    // when all four margins are equal. Set margins manually until fixed upstream.
    setContentsMargins(20, 20, 20, 20);

    const QString badgeStyle = QString("QLabel { background-color: %1; color: white; border-radius: 12px; }")
                                 .arg(palette().color(QPalette::Highlight).name());
    _pUi->lblStep1Nr->setStyleSheet(badgeStyle);
    _pUi->lblStep2Nr->setStyleSheet(badgeStyle);
    _pUi->lblStep3Nr->setStyleSheet(badgeStyle);

    connect(_pUi->btnClose, &QPushButton::clicked, this, &QDialog::reject);
    connect(_pUi->btnShowDocs, &QPushButton::clicked, this, &QuickStartDialog::showDocsRequested);
    connect(_pUi->btnLoadDemo, &QPushButton::clicked, this, &QuickStartDialog::loadDemoRequested);
}

QuickStartDialog::~QuickStartDialog()
{
    delete _pUi;
}

//! \brief Set the state of the "Don't show again" checkbox.
void QuickStartDialog::setDontShowAgain(bool bChecked)
{
    _pUi->chkDontShowAgain->setChecked(bChecked);
}

//! \brief Get the state of the "Don't show again" checkbox.
bool QuickStartDialog::dontShowAgain() const
{
    return _pUi->chkDontShowAgain->isChecked();
}
