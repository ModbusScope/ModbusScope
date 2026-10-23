
#ifndef TST_GRAPHDATAMODEL_H
#define TST_GRAPHDATAMODEL_H

#include <QObject>
#include <QStringList>

class GraphDataModel;

class TestGraphDataModel : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void addUsesDefaultExpression();
    void addUsesCustomDefaultExpression();
    void setDefaultExpressionUpdatesSubsequentAdds();
    void setDefaultExpressionIgnoresEmptyString();

    void addAssignsUniqueDefaultLabel();
    void addGraphDataWithoutLabelGetsUniqueDefaultLabel();
    void addGraphDataKeepsLabel();
    void addLabelListGivesEmptyLabelUniqueDefault();
    void addReusesLabelFreedByRename();
    void addReusesLabelFreedByRemove();
    void addUniqueLabelIsCaseSensitive();

    void activeCountZeroWhenEmpty();
    void activeCountReflectsAddedRegisters();
    void activeCountZeroWhenAllDeactivated();

    void setAllDataPassesQualities();
    void setAllDataDropsMisalignedQualities();

    void removeSignalsRemovesUnsortedIndices();
    void removeSignalsIgnoresDuplicateAndInvalidIndices();

    void rowAfterRemovalKeepsPositionInMiddle();
    void rowAfterRemovalSelectsLastRowAtEnd();
    void rowAfterRemovalReturnsNoneWhenEmpty();

    void moveSignalDown();
    void moveSignalUp();
    void moveSignalToLastRow();
    void moveSignalToFirstRow();
    void moveSignalToSamePositionDoesNothing();

private:
    static QStringList labels(const GraphDataModel& model);
    static void addLabels(GraphDataModel& model, const QStringList& labelList);
};

#endif /* TST_GRAPHDATAMODEL_H */
