
#ifndef TEST_DIAGNOSTIC_H__
#define TEST_DIAGNOSTIC_H__

#include <QObject>

class TestDiagnostic: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void fullConstructor();

    void setCategory();
    void setSeverity();
    void setTimeStamp();
    void setMessage();

    void severityString();
    void categoryString();

private:

};

#endif /* TEST_DIAGNOSTIC_H__ */
