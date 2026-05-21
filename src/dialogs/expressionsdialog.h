#ifndef EXPRESSIONSDIALOG_H
#define EXPRESSIONSDIALOG_H

#include "models/graphdatamodel.h"
#include "util/expressionchecker.h"

#include <QDialog>
#include <QJsonObject>
#include <QStringList>

/* Forward declarations */
class AdapterManager;
class ExpressionHighlighting;

namespace Ui {
class ExpressionsDialog;
}

class ExpressionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExpressionsDialog(GraphDataModel* pGraphDataModel,
                               GraphIdx idx,
                               AdapterManager* pAdapterManager,
                               QWidget* parent = nullptr);
    ~ExpressionsDialog();

private slots:
    void handleExpressionChange();
    void handleInputChange();
    void handleCancel();
    void handleAccept();
    void handleResultReady(bool valid);
    void handleExpressionHelpResult(const QString& helpText);
    void handleDescribeDataPointResult(const QJsonObject& result);

private:
    void startNextDescribe();

    Ui::ExpressionsDialog* _pUi;

    GraphIdx _graphIdx;

    GraphDataModel* _pGraphDataModel;
    AdapterManager* _pAdapterManager;

    ExpressionChecker _expressionChecker;
    ExpressionHighlighting* _pHighlighter;

    bool _bUpdating;

    QStringList _pendingDescribeAddresses;
    qint32 _nextDescribeRow = 0;
};

#endif // EXPRESSIONSDIALOG_H
