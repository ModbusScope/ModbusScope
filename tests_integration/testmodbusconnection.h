
#include <QObject>

class TestModbusConnection: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void connectionSuccess();
    void connectionFail();
    void connectionSuccesAfterFail();
};
