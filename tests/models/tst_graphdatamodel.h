
#ifndef TST_GRAPHDATAMODEL_H
#define TST_GRAPHDATAMODEL_H

#include <QObject>

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

    void activeCountZeroWhenEmpty();
    void activeCountReflectsAddedRegisters();
    void activeCountZeroWhenAllDeactivated();
};

#endif /* TST_GRAPHDATAMODEL_H */
