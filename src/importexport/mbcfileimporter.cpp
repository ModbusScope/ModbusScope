#include "mbcfileimporter.h"
#include "mbcregisterdata.h"
#include "util.h"


MbcFileImporter::MbcFileImporter(QString * pMbcFileContent) : QObject(nullptr)
{
    parseRegisters(pMbcFileContent);
}

QList <MbcRegisterData> MbcFileImporter::registerList()
{
    return _registerList;
}

QStringList MbcFileImporter::tabList()
{
    return _tabList;
}

void MbcFileImporter::parseRegisters(QString* pMbcFileContent)
{
    bool bRet = true;

    /* Clear register and tab list */
    _registerList.clear();
    _tabList.clear();

    QDomDocument domDocument;
    QString errorStr;
    qint32 errorLine;
    qint32 errorColumn;

    if (domDocument.setContent(*pMbcFileContent, true, &errorStr, &errorLine, &errorColumn))
    {
        QDomElement root = domDocument.documentElement();
        if (root.tagName().toLower().trimmed() == MbcFileDefinitions::cModbusControlTag)
        {
            QDomElement tag = root.firstChildElement();
            while (!tag.isNull())
            {
                if (tag.tagName().toLower().trimmed() == MbcFileDefinitions::cTabTag)
                {
                    bRet = parseTabTag(tag);
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

    if (bRet == false)
    {
        _registerList.clear();
        _tabList.clear();
    }
}

bool MbcFileImporter::parseTabTag(const QDomElement &element)
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
            _tabList.append(child.text());
            bFoundName = true;
        }
        else if (child.tagName().toLower().trimmed() == MbcFileDefinitions::cVarTag)
        {
            if (bFoundName)
            {
                bRet = parseVarTag(child, _tabList.size() - 1);
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

bool MbcFileImporter::parseVarTag(const QDomElement &element, qint32 tabIdx)
{
    bool bRet = true;

    QString name;
    QString addr;
    QString type;
    QString rw;
    QString decimals;
    MbcRegisterData modbusRegister;

    modbusRegister.setTabIdx(tabIdx);
    modbusRegister.setRegisterAddress(0);
    modbusRegister.setUnsigned(false);
    modbusRegister.setName(QString());
    modbusRegister.set32Bit(false);
    modbusRegister.setDecimals(0);

    QDomElement child = element.firstChildElement();

    while (!child.isNull())
    {
        if (child.tagName().toLower().trimmed() == MbcFileDefinitions::cRegisterTag)
        {
            addr = child.text().toLower().trimmed();
        }
        else if (child.tagName().toLower().trimmed() == MbcFileDefinitions::cTextTag)
        {
            name = child.text();
        }
        else if (child.tagName().toLower().trimmed() == MbcFileDefinitions::cTypeTag)
        {
            type = child.text().toLower().trimmed();
        }
        else if (child.tagName().toLower().trimmed() == MbcFileDefinitions::cReadWrite)
        {
             rw = child.text().toLower().trimmed();
        }
        else if (child.tagName().toLower().trimmed() == MbcFileDefinitions::cDecimals)
        {
             decimals = child.text();
        }
        else
        {
            // unknown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    /* Check for empty tag or unsupported 32 bit register */
    if (
            !name.isEmpty()
            || !addr.isEmpty()
            || !type.isEmpty()
            || !rw.isEmpty()
            || !decimals.isEmpty()
    )
    {
        /* Obligated */
        if (name.isEmpty())
        {
            bRet = false;
        }
        else
        {
            modbusRegister.setName(name);
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
                if (_nextRegisterAddr > 0)
                {
                    modbusRegister.setRegisterAddress(static_cast<quint16>(_nextRegisterAddr));
                }
                else
                {
                    bRet = false;
                }
            }
            else
            {
                modbusRegister.setRegisterAddress(static_cast<quint16>(addr.toUInt(&bRet)));
            }
        }

        /* Obligated */
        if (rw.isEmpty())
        {
            bRet = false;
        }
        else
        {
            if (rw.contains("r"))
            {
                modbusRegister.setReadable(true);
            }
            else
            {
                modbusRegister.setReadable(false);
            }
        }

        /* Optional */
        if (bRet)
        {
            if (type.right(2) == "32")
            {
                modbusRegister.set32Bit(true);
            }
            modbusRegister.setUnsigned(isUnsigned(type));
        }

        /* optional */
        if (bRet)
        {
            if (!decimals.isEmpty())
            {
                modbusRegister.setDecimals(static_cast<quint16>(decimals.toUInt(&bRet)));
            }
        }

        if (bRet)
        {
            /* Save register in list */
            _registerList.append(modbusRegister);

            if (modbusRegister.is32Bit())
            {               
                /* Increment address with 2 */
                _nextRegisterAddr = static_cast<qint32>(modbusRegister.registerAddress()) + 2;
            }
            else
            {
                /* Increment address with 1 */
                _nextRegisterAddr = static_cast<qint32>(modbusRegister.registerAddress()) + 1;
            }
        }
        else
        {
            Util::showError(tr("A tag is not present or value is not valid.\n\nName: %1\nRegister address: %2\nType: %3\nDecimals: %4")
                                .arg(name)
                                .arg(addr)
                                .arg(type)
                                .arg(decimals)
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
