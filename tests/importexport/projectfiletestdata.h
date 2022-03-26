
#include <QObject>

class ProjectFileTestData: public QObject
{
    Q_OBJECT

public:
    static QString cLegacyRegExpressions;
    static QString cBothLegacyAndNewRegExpressions;
    static QString cNewRegExpressions;
    static QString cDataLevel3Expressions;

    static QString cConnLegacySingle;
    static QString cConnLegacyDual;

    static QString cConnSerial;
    static QString cConnMixedMulti;
    static QString cConnEmpty;

private:

};
