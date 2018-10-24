
#include <QObject>

class TestModbusConnection: public QObject
{
    Q_OBJECT
private slots:
    void connectionSuccess();
};
