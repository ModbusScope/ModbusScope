#ifndef EXPRESSIONSDIALOG_H
#define EXPRESSIONSDIALOG_H

#include <QDialog>
#include "graphdatahandler.h"
#include "graphdatamodel.h"

/* Forward declaration */
class ExpressionHighlighting;

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
    void handleDataReady(ResultDoubleList resultList);

private:

    Ui::ExpressionsDialog*_pUi;

    qint32 _graphIdx;

    GraphDataModel* _pGraphDataModel;
    GraphDataModel _localGraphDataModel;
    GraphDataHandler _graphDataHandler;

    ExpressionHighlighting *_pHighlighter;

    bool _bUpdating;
    
};

#endif // EXPRESSIONSDIALOG_H
