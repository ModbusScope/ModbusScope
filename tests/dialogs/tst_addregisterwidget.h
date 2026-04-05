
#ifndef TST_ADDREGISTERWIDGET_H
#define TST_ADDREGISTERWIDGET_H

#include "models/graphdata.h"
#include "models/settingsmodel.h"

#include <QJsonObject>
#include <QObject>

class AddRegisterWidget;

class TestAddRegisterWidget : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void registerDefault();
    void registerType();
    void registerObjectType();
    void registerDevice();
    void registerValueAxis();

private:
    void pushOk();
    void addRegister(GraphData& graphData);
    static QJsonObject buildAddressSchema();
    static QJsonObject buildTestRegisterSchema();

    SettingsModel _settingsModel;
    AddRegisterWidget* _pRegWidget{ nullptr };
};

#endif // TST_ADDREGISTERWIDGET_H
