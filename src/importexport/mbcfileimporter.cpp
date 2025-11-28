#include "mbcfileimporter.h"

#include "importexport/mbcregisterdata.h"
#include "util/util.h"

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
    QDomDocument::ParseResult result =
      domDocument.setContent(*pMbcFileContent, QDomDocument::ParseOption::UseNamespaceProcessing);
    if (result)
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
                          .arg(result.errorLine)
                          .arg(result.errorColumn)
                          .arg(result.errorMessage));
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
            // unknown tag: ignore
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
    QString strType;
    QString rw;
    QString decimals;
    MbcRegisterData modbusRegister;

    modbusRegister.setTabIdx(tabIdx);

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
            strType = child.text().toLower().trimmed();
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
            || !strType.isEmpty()
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
                    modbusRegister.setRegisterAddress(_nextRegisterAddr);
                }
                else
                {
                    bRet = false;
                }
            }
            else
            {
                modbusRegister.setRegisterAddress(static_cast<quint32>(addr.toUInt(&bRet)));
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
            bool bOk;
            ModbusDataType::Type type = ModbusDataType::convertMbcString(strType, bOk);

            if (bOk)
            {
                modbusRegister.setType(type);
            }
            else
            {
                bRet = false;
            }
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

            if (ModbusDataType::is32Bit(modbusRegister.type()))
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
                                .arg(name, addr, strType, decimals)
                            );
        }

    }
    else
    {
        /* Empty var tag or 32 bit register: ignore */
    }

    return bRet;
}
