#ifndef ADDREGISTERDIALOG_H
#define ADDREGISTERDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include "graphdata.h"

class SettingsModel;

namespace Ui {
class AddRegisterDialog;
}

class AddRegisterDialog : public QDialog
{
    Q_OBJECT

    friend class TestAddRegisterDialog;

public:
    explicit AddRegisterDialog(SettingsModel* pSettingsModel, QWidget *parent = nullptr);
    ~AddRegisterDialog();

    GraphData graphData();

private:
    void done(int r);
    QString generateExpression();

    Ui::AddRegisterDialog * _pUi;

    SettingsModel* _pSettingsModel;

    QButtonGroup _bitGroup;
    QButtonGroup _signedGroup;

    GraphData _graphData;
};

#endif // ADDREGISTERDIALOG_H
