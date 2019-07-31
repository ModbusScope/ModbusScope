#ifndef MARKERINFODIALOG_H
#define MARKERINFODIALOG_H

#include <QDialog>
#include "guimodel.h"

namespace Ui {
class MarkerInfoDialog;
}

class MarkerInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MarkerInfoDialog(GuiModel *pGuiModel, QWidget *parent = nullptr);
    ~MarkerInfoDialog();

private slots:
    void checkBoxStatechanged(int state);

    void done(int r);

private:
    Ui::MarkerInfoDialog *_pUi;

    quint32 _expressionMask;

    GuiModel * _pGuiModel;
};

#endif // MARKERINFODIALOG_H
