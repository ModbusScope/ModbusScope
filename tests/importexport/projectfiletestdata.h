
#include <QObject>

class ProjectFileTestData: public QObject
{
    Q_OBJECT

public:
    static QString cLegacyRegExpressions;
    static QString cBothLegacyAndNewRegExpressions;
    static QString cNewRegExpressions;

    static QString cConnLegacySingle;
    static QString cConnLegacyDual;

private:

};
