#ifndef MBCFILEIMPORTER_H
#define MBCFILEIMPORTER_H

#include <QObject>
#include <QDomDocument>

#include "mbcregisterdata.h"

namespace MbcFileDefinitions
{

    /* Tag strings */
    const QString cModbusControlTag = QString("modbuscontrol");
    const QString cTabTag = QString("tab");

    const QString cTabNameTag = QString("name");
    const QString cVarTag = QString("var");

    const QString cRegisterTag = QString("reg");
    const QString cTextTag = QString("text");
    const QString cTypeTag = QString("type");
    const QString cReadWrite = QString("rw");
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
    bool isUnsigned(QString type);

    qint32 _nextRegisterAddr;

    QList <MbcRegisterData> _registerList;
    QStringList _tabList;
};

#endif // MBCFILEIMPORTER_H
