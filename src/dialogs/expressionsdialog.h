#ifndef EXPRESSIONSDIALOG_H
#define EXPRESSIONSDIALOG_H

#include "models/graphdatamodel.h"
#include "util/expressionchecker.h"

#include <QDialog>
#include <QHash>
#include <QJsonObject>
#include <QStringList>

/* Forward declarations */
class AdapterHub;
class AdapterManager;
class DataPointService;
class ExpressionHighlighting;
class QKeyEvent;
class QPushButton;
class SettingsModel;

namespace Ui {
class ExpressionsDialog;
}

class ExpressionsDialog : public QDialog
{
    Q_OBJECT

    friend class TestExpressionsDialog;

public:
    explicit ExpressionsDialog(GraphDataModel* pGraphDataModel,
                               GraphIdx idx,
                               AdapterHub* pAdapterHub,
                               SettingsModel* pSettingsModel,
                               DataPointService* pDataPointService,
                               QWidget* parent = nullptr);
    ~ExpressionsDialog();

protected:
    bool eventFilter(QObject* pWatched, QEvent* pEvent) override;
    void keyPressEvent(QKeyEvent* pEvent) override;

private slots:
    void handleExpressionChange();
    void handleInputChange();
    void handleCancel();
    void handleAccept();
    void handleResultReady(bool valid);
    void handleExpressionHelpResult(const QString& helpText);
    void handleDataPointDescribed(quint64 ticket, const QJsonObject& result);
    void handleDescribeFailed(quint64 ticket, const QString& error);
    void onHelpDeviceChanged(deviceId_t deviceId);

private:
    void requestDescriptions(const QStringList& addresses, const QList<deviceId_t>& deviceIds);
    void populateHelpDevices();
    void requestHelpForAdapter(const QString& adapterId);
    void setDescribeRowText(qint32 row, const QString& text);
    void setupButtons();
    static bool isEnterKey(const QKeyEvent* pEvent);
    void setupExpressionEditor();

    static constexpr int cExpressionEditorLines = 3;

    Ui::ExpressionsDialog* _pUi;

    GraphIdx _graphIdx;

    GraphDataModel* _pGraphDataModel;
    AdapterHub* _pAdapterHub;
    SettingsModel* _pSettingsModel;
    DataPointService* _pDataPointService;
    AdapterManager* _pHelpManager = nullptr;
    QPushButton* _pAcceptButton = nullptr;

    ExpressionChecker _expressionChecker;
    ExpressionHighlighting* _pHighlighter;

    bool _bUpdating;

    //! Whether the expression can be applied: no syntax error and every device exists.
    bool _bCanAccept{ false };

    //! Data point table row of every pending describe request, by ticket.
    QHash<quint64, qint32> _describeRowByTicket;
};

#endif // EXPRESSIONSDIALOG_H
