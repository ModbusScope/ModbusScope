#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

#include "graphdatamodel.h"
#include "guimodel.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(GuiModel * pGuiModel, GraphDataModel *pGraphDataModel, QWidget *parent = nullptr);
    ~RegisterDialog();

public slots:
    int execWithMbcImport();

private slots:
    void showImportDialog();
    void addRegisterRow();
    void removeRegisterRow();
    void activatedCell(QModelIndex modelIndex);
    void onRegisterInserted(const QModelIndex &parent, int first, int last);

private:
    int selectedRowAfterDelete(int deletedStartIndex, int rowCnt);
    static bool sortRegistersLastFirst(const QModelIndex &s1, const QModelIndex &s2);

    Ui::RegisterDialog * _pUi;

    GraphDataModel * _pGraphDataModel;
    GuiModel * _pGuiModel;
};

#endif // REGISTERDIALOG_H
