
#ifndef TST_DATAPOINT_H
#define TST_DATAPOINT_H

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

#endif /* TST_DATAPOINT_H */
