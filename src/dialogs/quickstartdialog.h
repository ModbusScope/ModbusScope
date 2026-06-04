#ifndef QUICKSTARTDIALOG_H
#define QUICKSTARTDIALOG_H

#include <QDialog>

namespace Ui {
class QuickStartDialog;
}

class QuickStartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuickStartDialog(QWidget* parent = nullptr);
    ~QuickStartDialog();

private:
    Ui::QuickStartDialog* _pUi;
};

#endif // QUICKSTARTDIALOG_H
