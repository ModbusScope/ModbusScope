#include "quickstartdialog.h"
#include "ui_quickstartdialog.h"

QuickStartDialog::QuickStartDialog(QWidget* parent) : QDialog(parent), _pUi(new Ui::QuickStartDialog)
{
    _pUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

QuickStartDialog::~QuickStartDialog()
{
    delete _pUi;
}
