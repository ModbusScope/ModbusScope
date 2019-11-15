
#include <QObject>
#include "mbcregisterdata.h"


class MbcTestData: public QObject
{
    Q_OBJECT

public:
    static QString cSingleTab;
    static QStringList cSingleTab_TabList;
    static QList <MbcRegisterData> cSingleTab_RegList;

    static QString cMultiTab;
    static QStringList cMultiTab_TabList;
    static QList <MbcRegisterData> cMultiTab_RegList;

private:

};
