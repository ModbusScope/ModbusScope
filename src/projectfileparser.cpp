
#include <QtWidgets>

#include "projectfileparser.h"

ProjectFileParser::ProjectFileParser()
{
    _msgBox.setWindowTitle(tr("ModbusScope project file load error"));
    _msgBox.setIcon(QMessageBox::Warning);
}


bool ProjectFileParser::parseFile(QIODevice *device, ProjectSettings *pSettings)
{
    bool bRet = true;
    QString errorStr;
    qint32 errorLine;
    qint32 errorColumn;

    if (!_domDocument.setContent(device, true, &errorStr, &errorLine, &errorColumn))
    {
        _msgBox.setText(tr("Parse error at line %1, column %2:\n%3")
                .arg(errorLine)
                .arg(errorColumn)
                .arg(errorStr));
        _msgBox.exec();

        bRet = false;
    }
    else
    {
        QDomElement root = _domDocument.documentElement();
        if (root.tagName() != "modbusscope")
        {
            _msgBox.setText(tr("The file is not a valid ModbusScope project file."));
            _msgBox.exec();
            bRet = false;
        }
        else
        {
            QDomElement tag = root.firstChildElement();
            while (!tag.isNull())
            {
                if (tag.tagName() == "modbus")
                {
                    bRet = parseModbusTag(tag, &pSettings->general);
                    if (!bRet)
                    {
                        break;
                    }
                }
                else if (tag.tagName() == "scope")
                {
                    bRet = parseScopeTag(tag, &pSettings->scope);
                    if (!bRet)
                    {
                        break;
                    }
                }
                tag = tag.nextSiblingElement();
            }
        }
    }

    return bRet;
}


bool ProjectFileParser::parseModbusTag(const QDomElement &element, GeneralSettings * pGeneralSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "ip")
        {
            pGeneralSettings->bIp = true;
            pGeneralSettings->ip = child.text();
        }
        else if (child.tagName() == "port")
        {
            pGeneralSettings->bPort = true;
            pGeneralSettings->port = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Port ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == "polltime")
        {
            pGeneralSettings->bPollTime = true;
            pGeneralSettings->pollTime = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Poll time ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();
                break;
            }
        }
        else if (child.tagName() == "slaveid")
        {
            pGeneralSettings->bSlaveId = true;
            pGeneralSettings->slaveId = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Slave id ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();

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


bool ProjectFileParser::parseScopeTag(const QDomElement &element, ScopeSettings *pScopeSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "var")
        {
            RegisterSettings registerData;
            bRet = parseVariableTag(child, &registerData);
            if (!bRet)
            {
                break;
            }
            pScopeSettings->registerList.append(registerData);
        }
        else
        {
            // unkown tag: ignore
        }
        child = child.nextSiblingElement();
    }

    return bRet;
}


bool ProjectFileParser::parseVariableTag(const QDomElement &element, RegisterSettings *pRegisterSettings)
{
    bool bRet = true;
    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "reg")
        {
            pRegisterSettings->address = child.text().toUInt(&bRet);
            if (!bRet)
            {
                _msgBox.setText(tr("Register ( %1 ) is not a valid number").arg(child.text()));
                _msgBox.exec();
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
