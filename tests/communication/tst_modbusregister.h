
#include <QObject>

class TestModbusRegister: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void constructor();
    void constructor_s32();
    void constructor_float32();
    void comparison();
    void copy();
    void description();


private:

};
