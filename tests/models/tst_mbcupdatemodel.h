#ifndef TEST_MBCUPDATEMODEL_H
#define TEST_MBCUPDATEMODEL_H

#include <QObject>

/* Forward declaration */
class GraphDataModel;
class SettingsModel;

class TestMbcUpdateModel : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void rowCount();
    void noUpdate();
    void tooMuchDifference();
    void updateText();
    void updateExpression();

    void columnCount();
    void userRoleHiddenWhenNoUpdate();
    void userRoleVisibleWhenUpdatePresent();
    void flagsNoUpdateIsDisabled();
    void flagsWithUpdateIsEnabled();
    void rowCountTracksGraphModel();
    void setMbcRegistersTriggersCheck();
    void updateTextToolTip();
    void updateExpressionToolTip();
    void displayColumnsCurrentValues();

private:
    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;
};

#endif /* TEST_MBCUPDATEMODEL_H */
