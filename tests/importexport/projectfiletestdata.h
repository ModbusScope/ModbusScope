
#include <QObject>

class ProjectFileTestData: public QObject
{
    Q_OBJECT

public:
    static QString cTooLowDataLevel;
    static QString cTooHighDataLevel;
    static QString cDataLevel3Expressions;
    static QString cDataLevel4Expressions;

    static QString cConnSerial;
    static QString cConnMixedMulti;
    static QString cConnEmpty;

    static QString cScaleDouble;
    static QString cValueAxis;

private:

};
