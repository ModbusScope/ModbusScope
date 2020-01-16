
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

    static QString cRegisterOptions;
    static QStringList cRegisterOptions_TabList;
    static QList <MbcRegisterData> cRegisterOptions_RegList;

    static QString cAutoincrement;
    static QStringList cAutoincrement_TabList;
    static QList <MbcRegisterData> cAutoincrement_RegList;

private:

};
