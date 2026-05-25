
#ifndef TST_EXPRESSIONSTATUS_H
#define TST_EXPRESSIONSTATUS_H

#include <QObject>

class GraphDataModel;
class SettingsModel;

class TestExpressionStatus : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void validConstantExpression_setsValidState();
    void syntaxErrorExpression_setsSyntaxErrorState();
    void registerExpression_noDevice_setsUnknownDeviceState();
    void afterRemoval_subsequentExpressionChange_setsCorrectState();

private:
    GraphDataModel* _pGraphDataModel = nullptr;
    SettingsModel* _pSettingsModel = nullptr;
};

#endif // TST_EXPRESSIONSTATUS_H
