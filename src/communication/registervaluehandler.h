#ifndef REGISTERVALUEHANDLER_H
#define REGISTERVALUEHANDLER_H

#include <QObject>

#include "modbusresult.h"

//Forward declaration
class GraphDataModel;
class SettingsModel;
class QMuParser;

class RegisterValueHandler : public QObject
{
    Q_OBJECT
public:
    RegisterValueHandler(GraphDataModel *pGraphDataModel, SettingsModel *pSettingsModel);

    void startRead();
    void finishRead();

    void processPartialResult(QMap<quint16, ModbusResult> partialResultMap, quint8 connectionId);

    QList<double> processedValues();
    QList<bool> successList();

    void activeGraphAddresList(QList<quint16> * pRegisterList, quint8 connectionId);

    void prepareForData();

signals:
    void registerDataReady(QList<bool> successList, QList<double> values);

private:

    GraphDataModel * _pGraphDataModel;
    SettingsModel * _pSettingsModel;

    QList<double> _processedValues;
    QList<bool> _successList;

    QList<quint16> _activeIndexList;
    QList<QMuParser*> _valueParsers;
};

#endif // REGISTERVALUEHANDLER_H
