
#include <QObject>


class TestProjectFileParser: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void initTestCase();
    void cleanup();

    void legacyRegExpressions();
    void newRegExpressions();
    void bothLegacyNewRegExpressions();
    void dataLevel3Expressions();

    void connLegacySingle();
    void connLegacyDual();
    void connSerial();
    void connMixedMulti();
    void connEmpty();

    void scaleDouble();
    void valueAxis();

private:

};
