
#include <QObject>

class TestDataFileParser: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void parseModbusScopeOldFormat();
    void parseModbusScopeNewFormat();

    void parseDatasetBe();
    void parseDatasetUs();

    void parseDatasetColumn2();
    void parseDatasetComment();
    void parseDatasetSigned();

    void parseDatasetAbsoluteDate();
    void parseDatasetTimeInSecond();

    void checkProgressSignal();

private:

};
