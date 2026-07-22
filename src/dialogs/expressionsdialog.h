#ifndef EXPRESSIONSDIALOG_H
#define EXPRESSIONSDIALOG_H

#include "models/graphdatamodel.h"
#include "util/expressionchecker.h"

#include <QDialog>
#include <QJsonObject>
#include <QStringList>

/* Forward declarations */
class AdapterHub;
class AdapterManager;
class ExpressionHighlighting;
class SettingsModel;

namespace Ui {
class ExpressionsDialog;
}

class ExpressionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExpressionsDialog(GraphDataModel* pGraphDataModel,
                               GraphIdx idx,
                               AdapterHub* pAdapterHub,
                               SettingsModel* pSettingsModel,
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
    void onHelpAdapterChanged(int index);

private:
    void startNextDescribe();
    void populateHelpAdapters();
    void requestHelpForAdapter(const QString& adapterId);
    AdapterManager* managerForDescribeRow(qint32 row) const;

    Ui::ExpressionsDialog* _pUi;

    GraphIdx _graphIdx;

    GraphDataModel* _pGraphDataModel;
    AdapterHub* _pAdapterHub;
    SettingsModel* _pSettingsModel;
    AdapterManager* _pDescribeManager = nullptr;
    AdapterManager* _pHelpManager = nullptr;

    ExpressionChecker _expressionChecker;
    ExpressionHighlighting* _pHighlighter;

    bool _bUpdating;

    QStringList _pendingDescribeAddresses;
    QList<deviceId_t> _pendingDescribeDeviceIds;
    qint32 _nextDescribeRow = 0;
};

#endif // EXPRESSIONSDIALOG_H
