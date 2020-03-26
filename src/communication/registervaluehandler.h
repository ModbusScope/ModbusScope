#ifndef REGISTERVALUEHANDLER_H
#define REGISTERVALUEHANDLER_H

#include <QObject>

#include "modbusresult.h"

//Forward declaration
class GraphDataModel;

class RegisterValueHandler
{
public:
    RegisterValueHandler(GraphDataModel *pGraphDataModel);

    void startRead();

    void processPartialResult(QMap<quint16, ModbusResult> partialResultMap, quint8 connectionId);

    QList<double> processedValues();
    QList<bool> successList();

    void activeGraphAddresList(QList<quint16> * pRegisterList, quint8 connectionId);

private:
    double processValue(quint32 graphIndex, quint32 value);

    GraphDataModel * _pGraphDataModel;

    QList<double> _processedValues;
    QList<bool> _successList;

    QList<quint16> _activeIndexList;
};

#endif // REGISTERVALUEHANDLER_H
