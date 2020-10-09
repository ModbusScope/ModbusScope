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
    explicit ExpressionsDialog(GraphDataModel * pGraphDataModel, qint32 idx, QWidget *parent = nullptr);
    ~ExpressionsDialog();

private slots:
    void handleExpressionChange();
    void handleInputChange();
    void handleCancel();
    void handleAccept();

private:

    void evaluateValue(QString strInput, QString &numOutput, QString &strTooltip);

    Ui::ExpressionsDialog *_pUi;

    qint32 _graphIdx;

    QMuParser _expressionParser;

    GraphDataModel * _pGraphDataModel;
};

#endif // EXPRESSIONSDIALOG_H
