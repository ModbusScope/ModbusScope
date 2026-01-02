
#include <QObject>

class TestModbusDataUnit : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void constructor_params();
    void constructor_from_address();
    void slave_getter_setter();

    void operator_equality();
    void operator_less_than();

private:
};
