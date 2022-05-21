
#include <QObject>

class TestModbusRegister: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void constructor();
    void comparison();
    void copy();
    void description();


private:

};
