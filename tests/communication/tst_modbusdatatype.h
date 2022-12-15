
#include <QObject>

class TestModbusDataType: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void is32bit();
    void isUnsigned();
    void isFloat();

    void convertString_data();
    void convertString();
    void convertStringUnknown();

private:

};
