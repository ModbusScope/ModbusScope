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
}

QuickStartDialog::~QuickStartDialog()
{
    delete _pUi;
}
