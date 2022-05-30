
#include <QObject>

#include "settingsmodel.h"

class AddRegisterDialog;

class TestAddRegisterDialog: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void test();

private:

    SettingsModel _settingsModel;
    AddRegisterDialog* _pRegDialog;

};
