#include "mbcfileimporter.h"

#include "util.h"
#include <QFile>
#include <QDebug>

MbcFileImporter::MbcFileImporter(QString filePath, QObject *parent) : QObject(parent)
{
    _file = filePath;
}

bool MbcFileImporter::parseRegisters(QList <MbcRegisterData> * pRegisterList)
{
    bool bRet = true;
    QFile file(_file);

    /* If we can't open it, let's show an error message. */
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        /* Clear register list */
        pRegisterList->clear();

        QString errorStr;
        qint32 errorLine;
        qint32 errorColumn;

        if (_domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn))
        {
            QDomElement root = _domDocument.documentElement();
            if (root.tagName().toLower().trimmed() == MbcFileDefinitions::cModbusControlTag)
            {
                QDomElement tag = root.firstChildElement();
                while (!tag.isNull())
                {
                    qDebug() << tag.tagName().toLower().trimmed();
                    if (tag.tagName().toLower().trimmed() == MbcFileDefinitions::cTabTag)
                    {
                        bRet = parseTabTag(tag, pRegisterList);
                        if (!bRet)
                        {
                            break;
                        }
                    }
                    else
                    {
                        /* Ignore other tags */
                    }

                    tag = tag.nextSiblingElement();
                }
            }
            else
            {
                Util::showError(tr("The file is not a valid ModbusControl project file."));
                bRet = false;
            }
        }
        else
        {
            Util::showError(tr("Parse error at line %1, column %2:\n%3")
                            .arg(errorLine)
                            .arg(errorColumn)
                            .arg(errorStr));
            bRet = false;
        }
    }

    return bRet;
}

bool MbcFileImporter::parseTabTag(const QDomElement &element, QList <MbcRegisterData> * pRegisterList)
{
    bool bRet = true;
    bool bFoundName = false;
    QString tabName;
    QDomElement child = element.firstChildElement();

    _nextRegisterAddr = -1;

    while (!child.isNull())
    {
        if (child.tagName().toLower().trimmed() == MbcFileDefinitions::cTabNameTag)
        {
            tabName = child.text();
            bFoundName = true;
        }
        else if (child.tagName().toLower().trimmed() == MbcFileDefinitions::cVarTag)
        {
            if (bFoundName)
            {
                bRet = parseVarTag(child, tabName, pRegisterList);
                if (!bRet)
                {
                    break;
                }
            }
            else
            {
                Util::showError(tr("No name tag in tab tag"));
                bRet = false;
                break;
            }
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}

bool MbcFileImporter::parseVarTag(const QDomElement &element, QString tabName, QList <MbcRegisterData> * pRegisterList)
{
    bool bRet = true;

    QString name;
    QString addr;
    QString type;
    MbcRegisterData modbusRegister;

    modbusRegister.tabName = tabName;
    modbusRegister.registerAddress = 0;
    modbusRegister.bUnsigned = false;
    modbusRegister.name = "";
    modbusRegister.bUint32 = false;

    QDomElement child = element.firstChildElement();

    while (!child.isNull())
    {
        if (child.tagName().toLower().trimmed() == MbcFileDefinitions::cRegisterTag)
        {
            addr = child.text();
        }
        else if (child.tagName().toLower().trimmed() == MbcFileDefinitions::cTextTag)
        {
            name = child.text();
        }
        else if (child.tagName().toLower().trimmed() == MbcFileDefinitions::cTypeTag)
        {
            type = child.text().toLower().trimmed();
        }
        else
        {
            // unknown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    /* Check for empty tag or unsopperted 32 bit register */
    if (
            !name.isEmpty()
            || !addr.isEmpty()
            || !type.isEmpty()
    )
    {
        /* Obligated */
        if (name.isEmpty())
        {
            bRet = false;
        }
        else
        {
            modbusRegister.name = name;
        }

        /* Obligated */
        if (bRet)
        {
            if (addr.isEmpty())
            {
                bRet = false;
            }
            else if (addr == "*")
            {
                if (_nextRegisterAddr == -1)
                {
                    bRet = false;
                }
                else
                {
                    modbusRegister.registerAddress = _nextRegisterAddr;
                }
            }
            else
            {
                modbusRegister.registerAddress = (quint16)addr.toUInt(&bRet);
            }
        }

        /* Optional */
        if (bRet)
        {
            if (type.right(2) == "32")
            {
                modbusRegister.bUint32 = true;
            }
            modbusRegister.bUnsigned = isUnsigned(type);
        }

        if (bRet)
        {
            /* Save register in list */
            pRegisterList->append(modbusRegister);

            if (modbusRegister.bUint32)
            {               
                /* Increment address with 2 */
                _nextRegisterAddr = modbusRegister.registerAddress + 2;
            }
            else
            {
                /* Increment address with 1 */
                _nextRegisterAddr = modbusRegister.registerAddress + 1;
            }
        }
        else
        {
            Util::showError(tr("A tag is not present or value is not valid.\n\nName: %1\nRegister address: %2\nType: %3\n")
                                .arg(name)
                                .arg(addr)
                                .arg(type)
                            );
        }

    }
    else
    {
        /* Empty var tag or 32 bit register: ignore */
    }

    return bRet;
}


bool MbcFileImporter::isUnsigned(QString type)
{
    bool bRet = true;

    if (type == "int16")
    {
        bRet = false;
    }

    return bRet;
}
