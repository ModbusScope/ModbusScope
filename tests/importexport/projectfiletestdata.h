
#include <QObject>

class ProjectFileTestData: public QObject
{
    Q_OBJECT

public:
    static QString cTooLowDataLevel;
    static QString cDataLevel3Expressions;

    static QString cConnSerial;
    static QString cConnMixedMulti;
    static QString cConnEmpty;

    static QString cScaleDouble;
    static QString cValueAxis;

private:

};
