#ifndef TST_SIGNALLISTDRAFT_H
#define TST_SIGNALLISTDRAFT_H

#include "models/settingsmodel.h"

#include <QObject>
#include <QStringList>

class GraphDataModel;

class TestSignalListDraft : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void draftCopiesSettings();
    void draftRowsDoNotShareSeriesWithSource();

    void applyWithoutChangesEmitsNothing();
    void renameKeepsData();
    void renameAndBackIsNoChange();
    void expressionChangeOnlyTouchesThatSignal();
    void deactivateThenDiscardKeepsSourceData();
    void deactivateThenApplyClearsSourceData();
    void deactivateAndReactivateIsNoChange();
    void addAppendsSignal();
    void removeRemovesSignalAndKeepsOthersData();
    void dragAndDropMoveThenApply();
    void combinedChangesFollowDraft();
    void secondApplyIsNoChange();
    void draftExpressionStateFollowsEdits();
    void moveAndMoveBackIsNoChange();
    void adjacentDropIsNoChange();
    void reorderOnlyKeepsData();
    void colorAxisAndVisibilityAreApplied();
    void activatedHiddenSignalStaysHidden();
    void editAfterApplyIsApplied();
    void emptyLabelIsKept();

private:
    void seedSignals(const QStringList& labels);
    static QStringList labels(const GraphDataModel* pModel);
    static void moveByDragAndDrop(GraphDataModel* pModel, int fromRow, int dropRow);
    qsizetype sampleCount(int row) const;

    SettingsModel _settingsModel;
    GraphDataModel* _pSource{ nullptr };
};

#endif // TST_SIGNALLISTDRAFT_H
