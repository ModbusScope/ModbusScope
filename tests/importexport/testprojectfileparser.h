
#include <QObject>


class TestProjectFileParser: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void legacyRegExpressions();
    void newRegExpressions();
    void bothLegacyNewRegExpressions();


    void connLegacySingle();
    void connLegacyDual();
    void connSerial();
    void connMixedMulti();
    void connEmpty();

private:

};
