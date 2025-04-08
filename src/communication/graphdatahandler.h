#ifndef GRAPHDATAHANDLER_H
#define GRAPHDATAHANDLER_H

#include "communication/modbusregister.h"
#include "util/qmuparser.h"
#include "util/result.h"

//Forward declaration
class GraphDataModel;

class GraphDataHandler : public QObject
{
    Q_OBJECT
public:
    GraphDataHandler();

    void processActiveRegisters(GraphDataModel *pGraphDataModel);
    void modbusRegisterList(QList<ModbusRegister>& registerList);

    QString expressionParseMsg(qint32 exprIdx) const;
    qint32 expressionErrorPos(qint32 exprIdx) const;
    QMuParser::ErrorType expressionErrorType(qint32 exprIdx) const;

public slots:
    void handleRegisterData(ResultDoubleList results);

signals:
    void graphDataReady(ResultDoubleList resultList);

private:

    GraphDataModel* _pGraphDataModel;

    QList<ModbusRegister> _registerList;
    QList<quint16> _activeIndexList;
    QList<QMuParser> _valueParsers;

};

#endif // GRAPHDATAHANDLER_H
