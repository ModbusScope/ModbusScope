#ifndef MBCFILEIMPORTER_H
#define MBCFILEIMPORTER_H

#include <QObject>
#include <QDomDocument>

#include "mbcregisterdata.h"

namespace MbcFileDefinitions
{

    /* Tag strings */
    const char cModbusControlTag[] = "modbuscontrol";
    const char cTabTag[] = "tab";

    const char cTabNameTag[] = "name";
    const char cVarTag[] = "var";

    const char cRegisterTag[] = "reg";
    const char cTextTag[] = "text";
    const char cTypeTag[] = "type";
    const char cReadWrite[] = "rw";
    const char cDecimals[] = "decimals";
}

class MbcFileImporter : public QObject
{
    Q_OBJECT
public:
    explicit MbcFileImporter(QString *mbcFileContent);

    QList <MbcRegisterData> registerList();
    QStringList tabList();

signals:

public slots:

private:
    void parseRegisters(QString* mbcFileContent);
    bool parseTabTag(const QDomElement &element);
    bool parseVarTag(const QDomElement &element, qint32 tabIdx);

    qint32 _nextRegisterAddr;

    QList <MbcRegisterData> _registerList;
    QStringList _tabList;
};

#endif // MBCFILEIMPORTER_H
