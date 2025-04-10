
#include "models/graphdata.h"
#include "models/settingsmodel.h"

#include <QObject>

class AddRegisterWidget;

class TestAddRegisterWidget: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void registerDefault();
    void registerType();
    void registerObjectType();
    void registerConnection();
    void registerValueAxis();

private:

    void pushOk();
    void addRegister(GraphData &graphData);

    SettingsModel _settingsModel;
    AddRegisterWidget* _pRegWidget;

};
