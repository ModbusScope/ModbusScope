#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

#include "registermodel.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(RegisterModel * pRegisterModel, QWidget *parent = 0);
    ~RegisterDialog();

private slots:
    void addRegisterRow();
    void removeRegisterRow();
    void updateColumns();

private:

    static bool sortRegistersLastFirst(const QModelIndex &s1, const QModelIndex &s2);

    Ui::RegisterDialog * _pUi;

    RegisterModel * _pRegisterModel;
};

#endif // REGISTERDIALOG_H
