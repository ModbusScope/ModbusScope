
#include <QObject>

/* Forward declaration */
class SettingsModel;
class GraphDataModel;
class MbcRegisterModel;
class MbcRegisterFilter;


class TestMbcRegisterFilter: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void noFilter();
    void tabFilter();
    void textFilter();
    void textAddressFilter();
    void tabTextFilter();

private:

    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;
    MbcRegisterModel* _pMbcRegisterModel;

    MbcRegisterFilter* _pFilterProxy;

};
