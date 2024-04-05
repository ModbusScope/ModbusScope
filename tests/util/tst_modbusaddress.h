
#include <QObject>

class TestModbusAddress: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void constructor_default();
    void constructor_address();
    void constructor_string_address();
    void constructor_type();
    void constructor_assignment();
    void constructor_copy();

    void addressFunctions_data();
    void addressFunctions();

    void to_string();
    void to_string_coil();
    void next_and_compare();

    void greater();
    void smaller();

    void sort_large_object_address();

private:


};
