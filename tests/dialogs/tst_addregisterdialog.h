
#include <QObject>

#include "settingsmodel.h"

class AddRegisterDialog;

class TestAddRegisterDialog: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void registerDefault();
    void register32Bit();
    void registerSignedRegister();
    void registerConnection();

private:

    void pushOk();

    SettingsModel _settingsModel;
    AddRegisterDialog* _pRegDialog;

};
