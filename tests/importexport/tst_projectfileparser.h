
#include <QObject>

class TestProjectFileParser: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void initTestCase();
    void cleanup();

    void tooLowDataLevel();
    void tooHighDataLevel();
    void dataLevel3Expressions();
    void dataLevel4Expressions();

    void connSerial();
    void connMixedMulti();
    void connEmpty();

    void scaleDouble();
    void valueAxis2Scaling();
    void valueAxis();

private:

};
