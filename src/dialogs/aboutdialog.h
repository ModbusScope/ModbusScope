#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include "updatenotify.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private slots:
    void openHomePage(void);
    void openLicense(void);
    void showVersionUpdate(UpdateNotify::UpdateState state);

private:

    Ui::AboutDialog * _pUi;

    UpdateNotify _updateNotify;
};

#endif // ABOUTDIALOG_H
