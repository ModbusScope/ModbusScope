
#include <QObject>

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

private:

    void pushOk();

    SettingsModel _settingsModel;
    AddRegisterWidget* _pRegWidget;

};
