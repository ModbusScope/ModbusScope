#ifndef IMPORTMBCDIALOG_H
#define IMPORTMBCDIALOG_H

#include <QDialog>
#include "mbcregistermodel.h"
#include "mbcregisterfilter.h"

/* Forward declaration */
class GuiModel;
class GraphDataModel;

namespace Ui {
class ImportMbcDialog;
}

class ImportMbcDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportMbcDialog(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel, MbcRegisterModel * pMbcRegisterModel, QWidget *parent = nullptr);
    ~ImportMbcDialog();

public slots:
    int exec(void);

private slots:
    void selectMbcFile();
    void registerDataChanged();

private:

    void updateMbcRegisters(QString filePath);

    Ui::ImportMbcDialog *_pUi;

    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;
    MbcRegisterModel * _pMbcRegisterModel;

    MbcRegisterFilter * _pTabProxyFilter;
};

#endif // IMPORTMBCDIALOG_H
