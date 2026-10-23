#ifndef EXPRESSIONSDIALOG_H
#define EXPRESSIONSDIALOG_H

#include "models/graphdatamodel.h"
#include "util/expressionchecker.h"

#include <QDialog>
#include <QStringList>

/* Forward declarations */
class AdapterHub;
class AdapterManager;
class DataPointDescriptionCache;
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
                               DataPointDescriptionCache* pDescriptionCache,
                               QWidget* parent = nullptr);
    ~ExpressionsDialog();

protected:
    bool eventFilter(QObject* pWatched, QEvent* pEvent) override;
    void keyPressEvent(QKeyEvent* pEvent) override;

private slots:
    void handleExpressionChange();
    void handleCellChanged(int row, int column);
    void handleInputChange();
    void handleCancel();
    void handleAccept();
    void handleResultReady(bool valid);
    void handleExpressionHelpResult(const QString& helpText);
    void handleDescriptionChanged(const QString& referenceText);
    void onHelpDeviceChanged(deviceId_t deviceId);

private:
    void showDescription(qint32 row);
    QString rowReferenceText(qint32 row) const;
    void populateHelpDevices();
    void requestHelpForAdapter(const QString& adapterId);
    void setupButtons();
    static bool isEnterKey(const QKeyEvent* pEvent);
    void setupExpressionEditor();

    static constexpr int cExpressionEditorLines = 3;

    //! Data roles of the data point column.
    static constexpr int cReferenceTextRole = Qt::UserRole;
    static constexpr int cDeviceIdRole = Qt::UserRole + 1;

    Ui::ExpressionsDialog* _pUi;

    GraphIdx _graphIdx;

    GraphDataModel* _pGraphDataModel;
    AdapterHub* _pAdapterHub;
    SettingsModel* _pSettingsModel;
    DataPointDescriptionCache* _pDescriptionCache;
    AdapterManager* _pHelpManager = nullptr;
    QPushButton* _pAcceptButton = nullptr;

    ExpressionChecker _expressionChecker;
    ExpressionHighlighting* _pHighlighter;

    bool _bUpdating;

    //! Whether the expression can be applied: no syntax error and every device exists.
    bool _bCanAccept{ false };
};

#endif // EXPRESSIONSDIALOG_H
