
#include <QObject>

class TestDataFileParser: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void parseModbusScopeOldFormat();
    void parseModbusScopeNewFormat();

private:

};
