#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include "updatenotify.h"
#include "basicgraphview.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(BasicGraphView * _pGraphView, QWidget *parent = 0);
    ~AboutDialog();

private slots:
    void openHomePage(void);
    void openLicense(void);
    void showVersionUpdate(UpdateNotify::UpdateState state, bool bDataLevelUpdate);

private:

    Ui::AboutDialog * _pUi;

    UpdateNotify _updateNotify;
};

#endif // ABOUTDIALOG_H
