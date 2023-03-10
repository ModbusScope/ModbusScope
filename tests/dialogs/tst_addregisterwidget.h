
#include <QObject>

#include "graphdata.h"
#include "settingsmodel.h"

class AddRegisterWidget;

class TestAddRegisterWidget: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void registerDefault();
    void registerType();
    void registerConnection();
    void registerValueAxis();

private:

    void pushOk();
    void addRegister(GraphData &graphData);

    SettingsModel _settingsModel;
    AddRegisterWidget* _pRegWidget;

};
