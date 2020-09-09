#ifndef EXPRESSIONSDIALOG_H
#define EXPRESSIONSDIALOG_H

#include <QDialog>
#include "qmuparser.h"

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
    void handleExpressionChange();
    void handleInputChange();
    void handleSaveExpression();
    void handleLoadExpression();
    void handleClose();
    void handleRegisterSelected(int idx);

private:

    QString evaluateValue(QString strInput);

    Ui::ExpressionsDialog *_pUi;

    QMuParser _expressionParser;

    GraphDataModel * _pGraphDataModel;
};

#endif // EXPRESSIONSDIALOG_H
