#ifndef EXPRESSIONSDIALOG_H
#define EXPRESSIONSDIALOG_H

#include <QDialog>
#include "graphdatahandler.h"
#include "graphdatamodel.h"

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
    void handleDataReady(QList<bool> successList, QList<double> values);

private:

    Ui::ExpressionsDialog*_pUi;

    qint32 _graphIdx;

    GraphDataModel* _pGraphDataModel;

    GraphDataModel _localGraphDataModel;
    GraphDataHandler _graphDataHandler;

    bool _bUpdating;
};

#endif // EXPRESSIONSDIALOG_H
