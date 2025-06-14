
#include "tst_scaledock.h"

#include "customwidgets/scaledock.h"
#include "ui_scaledock.h"

#include <QTest>

void TestScaleDock::init()
{
    _pScaleDock = new ScaleDock();
}

void TestScaleDock::cleanup()
{
    delete _pScaleDock;
}

void TestScaleDock::yAxis1SelectedOnBoot()
{
    // Make sure tab 0 is selected at boot
    QCOMPARE(_pScaleDock->_pUi->scaleTab->currentIndex(), 0u);
}

QTEST_MAIN(TestScaleDock)
