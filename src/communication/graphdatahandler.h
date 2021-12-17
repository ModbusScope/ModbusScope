#ifndef GRAPHDATAHANDLER_H
#define GRAPHDATAHANDLER_H

#include <QRegularExpression>
#include "modbusregister.h"
#include "result.h"

//Forward declaration
class GraphDataModel;
class QMuParser;

class GraphDataHandler : public QObject
{
    Q_OBJECT
public:
    GraphDataHandler();

    void processActiveRegisters(GraphDataModel *pGraphDataModel);
    void modbusRegisterList(QList<ModbusRegister>& registerList);

public slots:
    void handleRegisterData(QList<Result> results);

signals:
    void graphDataReady(QList<bool> successList, QList<double> values);

private:

    GraphDataModel* _pGraphDataModel;

    QList<ModbusRegister> _registerList;
    QList<quint16> _activeIndexList;
    QList<QMuParser*> _valueParsers;

};

#endif // GRAPHDATAHANDLER_H
