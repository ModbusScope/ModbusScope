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

    void setDontShowAgain(bool bChecked);
    bool dontShowAgain() const;

signals:
    void showDocsRequested();

private:
    Ui::QuickStartDialog* _pUi;
};

#endif // QUICKSTARTDIALOG_H
