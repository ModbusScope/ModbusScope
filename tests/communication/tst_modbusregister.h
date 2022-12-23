
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

    void processValue_16b_data();
    void processValue_16b();
    void processValue_s16b_data();
    void processValue_s16b();

    void processValue_32b_data();
    void processValue_32b();
    void processValue_s32b_data();
    void processValue_s32b();

    void processValue_f32b_data();
    void processValue_f32b();

private:

};
