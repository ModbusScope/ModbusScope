
#ifndef TEST_MBCUPDATEMODEL_H__
#define TEST_MBCUPDATEMODEL_H__

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

private:
    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;
};

#endif /* TEST_MBCUPDATEMODEL_H__ */
