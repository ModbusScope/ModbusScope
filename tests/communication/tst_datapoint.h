
#ifndef TEST_DATAPOINT_H__
#define TEST_DATAPOINT_H__

#include <QObject>

class TestDataPoint : public QObject
{
    Q_OBJECT

private slots:

    void init();
    void cleanup();

    void defaultConstructor();
    void constructorWithArgs();
    void copyConstructor();
    void assignmentOperator();
    void selfAssignment();
    void equalityOperator();
    void description();
    void dumpListToString();
    void dumpEmptyListToString();

private:
};

#endif /* TEST_DATAPOINT_H__ */
