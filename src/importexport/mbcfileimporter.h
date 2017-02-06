#ifndef MBCFILEIMPORTER_H
#define MBCFILEIMPORTER_H

#include <QObject>
#include <QDomDocument>

struct MbcRegisterData
{
    quint16 registerAddress;
    bool bUnsigned;
    QString name;
    QString tabName;
    bool bUint32;

};

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


    /* Constant values */
    //const quint32 cCurrentDataLevel = 2;

}

class MbcFileImporter : public QObject
{
    Q_OBJECT
public:
    explicit MbcFileImporter(QString filePath, QObject *parent = 0);

    bool parseRegisters(QList <MbcRegisterData> * pRegisterList);

signals:

public slots:

private:
    bool parseTabTag(const QDomElement &element, QList <MbcRegisterData> * pRegisterList);
    bool parseVarTag(const QDomElement &element, QString tabName, QList <MbcRegisterData> * pRegisterList);
    bool isUnsigned(QString type);

    QString _file;
    QDomDocument _domDocument;
    qint32 _nextRegisterAddr;
};

#endif // MBCFILEIMPORTER_H
