
#include <QtTest/QtTest>

#include "addregisterdialog.h"
#include "ui_addregisterdialog.h"

#include "tst_addregisterdialog.h"


void TestAddRegisterDialog::init()
{
    _pRegDialog = new AddRegisterDialog(&_settingsModel);
}

void TestAddRegisterDialog::cleanup()
{

}

void TestAddRegisterDialog::test()
{
    QCOMPARE(_pRegDialog->_pUi->lineName->text(), "Name of curve");
}

QTEST_MAIN(TestAddRegisterDialog)
