
#ifndef TEST_GRAPHDATAHANDLER_H__
#define TEST_GRAPHDATAHANDLER_H__

#include "util/result.h"

#include <QObject>

/* Forward declaration */
class GraphDataModel;
class SettingsModel;

class TestGraphDataHandler: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void error_data();
    void error();

    void sameRegisterDifferentType();

    void registerList();
    void manyInactiveRegisters();

    void graphData();
    void graphDataTwice();
    void graphData_fail();

private:

    void doHandleRegisterData(ResultDoubleList& modbusResults, QList<QVariant> &actRawData);

    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;

};

#endif /* TEST_GRAPHDATAHANDLER_H__ */
