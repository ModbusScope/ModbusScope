#include "graphdatahandler.h"

#include "datahandling/expressionparser.h"
#include "datahandling/qmuparser.h"
#include "models/graphdatamodel.h"
#include "util/scopelogging.h"

#include <QRegularExpression>

/*!
 * \param[in]     pGraphDataModel   Graph data model
 * \param[out]    registerList      List of modbus registers
 */
void GraphDataHandler::setupExpressions(GraphDataModel* pGraphDataModel, QList<ModbusRegister>& registerList)
{
    QStringList exprList;
    QList<ModbusRegister> regList;

    pGraphDataModel->activeGraphIndexList(&_activeIndexList);
    for(quint16 graphIdx: std::as_const(_activeIndexList))
    {
        exprList.append(pGraphDataModel->expression(graphIdx));
    }

    ExpressionParser exprParser(exprList);
    exprParser.modbusRegisters(regList);

    qCInfo(scopeComm) << "Active registers: " << ModbusRegister::dumpListToString(regList);

    QStringList processedExpList;
    exprParser.processedExpressions(processedExpList);

    _valueParsers.clear();

    for(const QString &expr: std::as_const(processedExpList))
    {
        _valueParsers.append(QMuParser(expr));
    }

    registerList = regList;
}

QString GraphDataHandler::expressionParseMsg(qint32 exprIdx) const
{
    if (exprIdx >= _valueParsers.size())
    {
        return QString();
    }

    return _valueParsers[exprIdx].msg();
}

qint32 GraphDataHandler::expressionErrorPos(qint32 exprIdx) const
{
    if (exprIdx >= _valueParsers.size())
    {
        return -1;
    }

    return _valueParsers[exprIdx].errorPos();
}

QMuParser::ErrorType GraphDataHandler::expressionErrorType(qint32 exprIdx) const
{
    if (exprIdx >= _valueParsers.size())
    {
        return QMuParser::ErrorType::SYNTAX;
    }

    return _valueParsers[exprIdx].errorType();
}

void GraphDataHandler::handleRegisterData(ResultDoubleList results)
{
    ResultDoubleList registerList;

    QMuParser::setRegistersData(results);

    for (auto& parser : _valueParsers)
    {
        ResultDouble result;

        if (parser.evaluate())
        {
            result.setValue(parser.value());
        }
        else
        {
            result.setError();

            auto msg = QString("Expression evaluation failed (%1): expression %2")
                         .arg(parser.msg(), parser.originalExpression());

            qCWarning(scopeComm) << msg;
        }

        registerList.append(result);
    }

    emit graphDataReady(registerList);
}



