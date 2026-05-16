
#ifndef TST_GRAPHDATASTORE_H
#define TST_GRAPHDATASTORE_H

#include <QObject>

class TestGraphDataStore : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void insertIncreasesSize();
    void insertAssignsColorFromList();
    void insertPreservesExplicitColor();
    void eraseRemovesCorrectEntry();
    void clearEmptiesStore();
    void setActiveFalseRemovesFromActiveCount();
    void setActiveFalseClearsData();
    void setActiveTrueRestoresVisibility();
    void activeGraphIndexListReturnsSortedIndices();
    void convertToActiveGraphIndexRoundTrip();
    void setVisibleFalseOnSelectedGraphResetsSelection();
    void moveGraphRowReorders();

    void eraseSelectedGraphResetsSelection();
    void eraseGraphBeforeSelectedResetsSelection();
    void eraseGraphAfterSelectedResetsSelection();
    void clearAllGraphDataResetsSelection();
    void moveRowResetsSelection();

    void insertInactiveGraphDoesNotIncreaseActiveCount();
    void setActiveNoopDoesNotEmitSignal();
    void setSelectedGraphEmitsSignal();
    void setSelectedGraphOnInvisibleGraphIsIgnored();
    void setVisibleTrueEmitsSignal();
    void setVisibleFalseEmitsSignal();
    void setValueAxisEmitsSignal();
    void setExpressionEmitsSignal();
    void setExpressionStateEmitsSignal();
    void setLabelEmitsSignal();
    void setColorEmitsSignal();
};

#endif /* TST_GRAPHDATASTORE_H */
