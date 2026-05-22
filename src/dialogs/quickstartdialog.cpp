#include "quickstartdialog.h"
#include "ui_quickstartdialog.h"

QuickStartDialog::QuickStartDialog(QWidget* parent) : QDialog(parent), _pUi(new Ui::QuickStartDialog)
{
    _pUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    // Qt uic 6.8.3 bug: generates invalid setContentsMargins(20) (single-int overload)
    // when all four margins are equal. Set margins manually until fixed upstream.
    setContentsMargins(20, 20, 20, 20);
}

QuickStartDialog::~QuickStartDialog()
{
    delete _pUi;
}
