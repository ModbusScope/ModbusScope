
#ifndef TEST_MBCREGISTERMODEL_H__
#define TEST_MBCREGISTERMODEL_H__

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
    void disableAlreadyStagedRegisterAddress();
    void fillData();
    void reset();
    void selectedRegisterListAndCount();
    void selectedRegisterListAndCount32();

    void selectedRegisterListDecimals();

private:

    void fillModel(MbcRegisterModel * pMbcRegisterModel);

};

#endif /* TEST_MBCREGISTERMODEL_H__ */
