#ifndef TST_MBCREGISTERMODEL_H
#define TST_MBCREGISTERMODEL_H

#include <QObject>

/* Forward declaration */
class MbcRegisterModel;

class TestMbcRegisterModel: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void rowCount();
    void columnCount();
    void headerData();
    void flagsEnabled();
    void flagsDisabled();
    void setData();
    void fillData();
    void reset();
    void selectedRegisterListAndCount();
    void selectedRegisterListAndCount32();
    void selectedRegisterListDecimals();

    void selectAllWhenUnchecked();
    void selectAllWhenChecked();
    void selectAllWhenMixedChecked();
    void selectAllSkipNonReadable();

    void setDataOnDisabledRowDoesNothing();
    void setDataInvalidIndexReturnsFalse();
    void headerDataCheckState();
    void setHeaderDataCheckState();
    void setHeaderDataPartiallyChecked();
    void dataInvalidIndexReturnsNull();
    void fillTwiceResetsFirst();
    void setSelectionstateEmptyListDoesNothing();
    void selectedRegisterListEmpty();

private:

    void fillModel(MbcRegisterModel * pMbcRegisterModel);

};

#endif /* TST_MBCREGISTERMODEL_H */
