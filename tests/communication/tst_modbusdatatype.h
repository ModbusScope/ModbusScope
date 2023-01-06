
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

    void convertMbcString_data();
    void convertMbcString();

    void convertStringUnknown();

    void typeString_data();
    void typeString();

    void convertSettings_data();
    void convertSettings();

private:

};
