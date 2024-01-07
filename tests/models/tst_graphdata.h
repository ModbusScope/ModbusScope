
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
    void setExpressionResetsStatus();

private:

};

#endif /* TEST_GRAPHDATA_H__ */
