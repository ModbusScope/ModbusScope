#ifndef GRAPHDATAHANDLER_H
#define GRAPHDATAHANDLER_H

#include <QObject>
#include "modbusregister.h"

//Forward declaration
class GraphDataModel;
class SettingsModel;
class QMuParser;

class GraphDataHandler : public QObject
{
    Q_OBJECT
public:
    GraphDataHandler(GraphDataModel *pGraphDataModel, SettingsModel *pSettingsModel);

    void prepareForData();

    void modbusRegisterList(QList<ModbusRegister>& registerList);

public slots:
    void handleRegisterData(QList<bool> successList, QList<double> values);

signals:
    void graphDataReady(QList<bool> successList, QList<double> values);

private:

    GraphDataModel* _pGraphDataModel;
    SettingsModel* _pSettingsModel;

    QList<quint16> _activeIndexList;
    QList<QMuParser*> _valueParsers;

};

#endif // GRAPHDATAHANDLER_H
