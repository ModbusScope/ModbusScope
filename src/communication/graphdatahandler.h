#ifndef GRAPHDATAHANDLER_H
#define GRAPHDATAHANDLER_H

#include <QRegularExpression>
#include "modbusregister.h"
#include "result.h"
#include "qmuparser.h"

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

public slots:
    void handleRegisterData(QList<Result<double> > results);

signals:
    void graphDataReady(QList<bool> successList, QList<double> values);

private:

    GraphDataModel* _pGraphDataModel;

    QList<ModbusRegister> _registerList;
    QList<quint16> _activeIndexList;
    QList<QMuParser> _valueParsers;

};

#endif // GRAPHDATAHANDLER_H
