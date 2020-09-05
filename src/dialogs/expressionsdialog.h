#ifndef EXPRESSIONSDIALOG_H
#define EXPRESSIONSDIALOG_H

#include <QDialog>

/* Forward declaration */
class GraphDataModel;

namespace Ui {
class ExpressionsDialog;
}

class ExpressionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExpressionsDialog(GraphDataModel * pGraphDataModel, QWidget *parent = nullptr);
    ~ExpressionsDialog();

private slots:

    void handleSaveExpression();
    void handleLoadExpression();
    void handleClose();
    void handleRegisterSelected(int idx);

private:
    Ui::ExpressionsDialog *_pUi;

    GraphDataModel * _pGraphDataModel;
};

#endif // EXPRESSIONSDIALOG_H
