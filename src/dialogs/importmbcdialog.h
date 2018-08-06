#ifndef IMPORTMBCDIALOG_H
#define IMPORTMBCDIALOG_H

#include <QDialog>
#include "mbcregistermodel.h"
#include "mbcregisterfilter.h"

/* Forward declaration */
class GuiModel;
class GraphData;
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
    int exec(QString mbcPath);

private slots:
    void selectMbcFile();
    void registerDataChanged();

private:

    bool updateMbcRegisters();

    Ui::ImportMbcDialog *_pUi;

    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;
    MbcRegisterModel * _pMbcRegisterModel;

    MbcRegisterFilter * _pTabProxyFilter;

    QString _mbcFilePath;
};

#endif // IMPORTMBCDIALOG_H
