
#ifndef TEST_ERRORLOGMODEL_H__
#define TEST_ERRORLOGMODEL_H__

#include <QObject>

class TestErrorLogModel: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void addClear();
    void headerData();
    void data();
    void dataCategory();
    void flags();
    void addItem();

private:

};

#endif /* TEST_ERRORLOGMODEL_H__ */
