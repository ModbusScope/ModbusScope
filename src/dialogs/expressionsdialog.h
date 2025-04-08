#ifndef EXPRESSIONSDIALOG_H
#define EXPRESSIONSDIALOG_H

#include "models/graphdatamodel.h"
#include "util/expressionchecker.h"

#include <QDialog>

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
    void handleResultReady(bool valid);

private:

    Ui::ExpressionsDialog*_pUi;

    qint32 _graphIdx;

    GraphDataModel* _pGraphDataModel;

    ExpressionChecker _expressionChecker;
    ExpressionHighlighting *_pHighlighter;

    bool _bUpdating;
    
};

#endif // EXPRESSIONSDIALOG_H
