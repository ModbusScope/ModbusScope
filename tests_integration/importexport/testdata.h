
#include <QObject>

class TestData: public QObject
{
    Q_OBJECT

public:
    static QString cModbusScopeOldFormat;
    static QString cModbusScopeNewFormat;

    static QString cDatasetBe;
    static QString cDatasetUs;

    static QString cDatasetColumn2;
    static QString cDatasetComment;

    static QString cDatasetSigned;

    static QString cDatasetAbsoluteDate;

    static QString cDatasetTimeInSecond;

private:

};
