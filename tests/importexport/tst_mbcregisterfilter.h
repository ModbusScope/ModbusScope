
#include <QObject>

/* Forward declaration */
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
    MbcRegisterModel* _pMbcRegisterModel;

    MbcRegisterFilter* _pFilterProxy;

};
