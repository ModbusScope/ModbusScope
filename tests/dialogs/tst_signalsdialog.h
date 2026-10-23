
#ifndef TST_SIGNALSDIALOG_H
#define TST_SIGNALSDIALOG_H

#include "../mocks/mockadapterhub.h"
#include "../mocks/mockdatapointmanager.h"
#include "models/settingsmodel.h"

#include <QJsonObject>
#include <QMessageBox>
#include <QObject>

class GraphDataModel;
class SignalEditorPanel;
class SignalsDialog;

class TestSignalsDialog : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void dataPointLimitWarningHiddenWhenWithinLimit();
    void dataPointLimitWarningShownWhenExceeded();
    void dataPointLimitWarningHiddenWhenSignalDeactivated();
    void dataPointLimitWarningHiddenWhenAdapterReportsNoLimit();

    void buttonBoxHasOkAndCancel();
    void tableEditsDraftNotSource();
    void cancelDiscardsChanges();
    void escapeDiscardsChanges();
    void cancelWithoutChangesClosesWithoutQuestion();
    void cancelQuestionKeepsDialogOpen();
    void closeWithOpenCellEditAsksQuestion();
    void okAppliesChanges();
    void okWithoutChangesLeavesSourceUntouched();

    void addButtonClearsSelectionAndFocusesName();
    void addedSignalScrolledIntoViewNotSelected();
    void enterInPanelDoesNotCloseDialog();
    void deleteInNameFieldKeepsRows();
    void cancelDiscardsPanelAdds();
    void minimumSizeKeepsTableAndPanelUsable();
    void panelOpensWiderThanItsMinimum();
    void widerDialogWidensOnlyTheTable();

    void dataPointColumnShowsDescription();
    void dragFromDataPointColumnReordersSignals();
    void clickOnDataPointColumnSelectsRowForRemove();

    void removeMiddleRowSelectsNextRow();
    void removeLastRowSelectsNewLastRow();
    void removeOnlyRowLeavesNoSelection();

    void selectingRowEditsSignalInPanel();
    void cancelDiscardsPanelEdits();
    void okAppliesPanelEdits();
    void editModeFollowsMovedRow();

private:
    static QJsonObject buildTestRegisterSchema();
    static QJsonObject buildDescribeWithMaxRegisters(int maxRegisters);
    void seedSignals(int count);
    void answerNextQuestion(QMessageBox::StandardButton button);
    void clickInActiveMessageBox(QMessageBox::StandardButton button);
    void clickCancel();
    void seedSamples();
    void editDraft();
    GraphDataModel* draftModel() const;
    SignalEditorPanel* panel() const;
    void openWithReadyAdapter(int signalCount);
    void addSignalThroughPanel(const QString& expression);
    QStringList draftExpressions() const;
    void removeRowThroughButton(int row);
    QList<int> selectedRows() const;
    void editFirstSignalAddressThroughPanel(int address, const QString& builtReference);

    SettingsModel _settingsModel;
    GraphDataModel* _pGraphDataModel{ nullptr };
    MockAdapterHub* _pMockHub{ nullptr };
    MockDataPointManager* _pMockDataPointManager{ nullptr };
    SignalsDialog* _pDialog{ nullptr };

    //! Number of message boxes answered by clickInActiveMessageBox() in the current test.
    int _answeredQuestions{ 0 };
};

#endif // TST_SIGNALSDIALOG_H
