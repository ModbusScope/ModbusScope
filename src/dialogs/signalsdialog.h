#ifndef SIGNALSDIALOG_H
#define SIGNALSDIALOG_H

#include "customwidgets/centeredbox.h"
#include "util/graphindex.h"

#include <QDialog>

/* Forward declaration */
class GraphDataModel;
class AdapterHub;
class DataPointDescriptionCache;
class DataPointService;
class SettingsModel;
class SignalEditorPanel;
class SignalListDraft;
class SignalTableProxyModel;
class ValueAxisDelegate;
class ActionButtonDelegate;

namespace Ui {
class SignalsDialog;
}

class SignalsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignalsDialog(GraphDataModel* pGraphDataModel,
                           SettingsModel* pSettingsModel,
                           AdapterHub* pAdapterHub,
                           QWidget* parent = nullptr);
    ~SignalsDialog();

public slots:
    void accept() override;
    void reject() override;

private slots:
    void startAddSignal();
    void removeSelectedSignals();
    void activatedCell(QModelIndex modelIndex);
    void onSignalInserted(const QModelIndex& parent, int first, int last);
    void handleExpressionEdit(const QModelIndex& index);
    void openExpressionDialog(GraphIdx graphIdx);
    void onSelectionChanged();
    void updateDataPointLimitIndication();

private:
    void setupEditorPanel();
    void setupDataPointLimitIndication();
    QString dataPointLimitWarningMessage() const;

    Ui::SignalsDialog* _pUi;

    SignalListDraft* _pDraft;
    SettingsModel* _pSettingsModel;
    DataPointService* _pDataPointService;
    DataPointDescriptionCache* _pDescriptionCache;
    SignalTableProxyModel* _pTableModel;
    SignalEditorPanel* _pPanel{ nullptr };

    CenteredBoxProxyStyle _centeredBoxStyle;

    ValueAxisDelegate* _pValueAxisDelegate;
    ActionButtonDelegate* _pExpressionDelegate;

    static constexpr int cMinimumWidth = 680;
    static constexpr int cMinimumHeight = 400;
    static constexpr int cPanelWidth = 400;
};

#endif // SIGNALSDIALOG_H
