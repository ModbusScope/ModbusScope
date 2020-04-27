
#ifndef TEST_ERRORLOG_H__
#define TEST_ERRORLOG_H__

#include <QObject>

class TestErrorLog: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void fullConstructor();
    void setSeverity();
    void setTimeStamp();
    void setMessage();
    void categoryString();

private:

};

#endif /* TEST_ERRORLOG_H__ */
