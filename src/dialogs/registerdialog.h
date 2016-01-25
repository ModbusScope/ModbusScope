#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

#include "graphdatamodel.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(GraphDataModel *pGraphDataModel, QWidget *parent = 0);
    ~RegisterDialog();

private slots:
    void done(int r);
    void addRegisterRow();
    void removeRegisterRow();
    void activatedCell(QModelIndex modelIndex);

private:

    static bool sortRegistersLastFirst(const QModelIndex &s1, const QModelIndex &s2);

    Ui::RegisterDialog * _pUi;

    GraphDataModel * _pGraphDataModel;
};

#endif // REGISTERDIALOG_H
