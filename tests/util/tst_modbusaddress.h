
#include <QObject>

class TestModbusAddress: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void constructor_default();
    void constructor_address();
    void constructor_type();
    void constructor_assignment();
    void constructor_copy();

    void get_address();

    void to_string();
    void next_and_compare();

    void greater();
    void smaller();

private:


};
