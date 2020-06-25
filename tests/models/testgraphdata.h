
#ifndef TEST_GRAPHDATA_H__
#define TEST_GRAPHDATA_H__

#include <QObject>

class TestGraphData: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void setLabel();

    void setExpression_data();
    void setExpression();

private:

};

#endif /* TEST_GRAPHDATA_H__ */
