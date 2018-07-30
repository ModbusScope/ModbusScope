#ifndef IMPORTMBCDIALOG_H
#define IMPORTMBCDIALOG_H

#include <QDialog>
#include "mbcregistermodel.h"

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
    void tabFilterChanged(const QString &text);

private:

    bool updateMbcRegisters();

    Ui::ImportMbcDialog *_pUi;

    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;
    MbcRegisterModel * _pMbcRegisterModel;

    QString _mbcFilePath;

    static const QString _cTabFilterAll;
};

#endif // IMPORTMBCDIALOG_H
