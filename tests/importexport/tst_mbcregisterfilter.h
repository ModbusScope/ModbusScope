#ifndef TST_MBCREGISTERFILTER_H
#define TST_MBCREGISTERFILTER_H

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

    void tabNoFilterShowsAll();
    void tabFilterHidesOtherTabs();
    void textFilterCaseInsensitive();
    void addressFilterPartialMatch();
    void emptyModelAcceptsNothing();
    void outOfRangeRowRejected();

private:
    MbcRegisterModel* _pMbcRegisterModel;

    MbcRegisterFilter* _pFilterProxy;

};

#endif // TST_MBCREGISTERFILTER_H
