
#include <QtTest/QtTest>

#include "addregisterwidget.h"
#include "ui_addregisterwidget.h"

#include "tst_addregisterwidget.h"


void TestAddRegisterWidget::init()
{
    _pRegWidget = new AddRegisterWidget(&_settingsModel);
}

void TestAddRegisterWidget::cleanup()
{
    delete _pRegWidget;
}

void TestAddRegisterWidget::registerDefault()
{
    _pRegWidget->_pUi->lineName->selectAll();
    QTest::keyClicks(_pRegWidget->_pUi->lineName, "Register 1");

    _pRegWidget->_pUi->spinAddress->selectAll();
    QTest::keyClicks(_pRegWidget->_pUi->spinAddress, "41000");

    pushOk();
#if 0
    auto graphData = _pRegWidget->graphData();
    QCOMPARE(graphData.label(), "Register 1");
    QCOMPARE(graphData.expression(), "${41000}");
    QVERIFY(graphData.isActive());
#endif
}

void TestAddRegisterWidget::registerType()
{
    QTest::keyClick(_pRegWidget->_pUi->cmbType, Qt::Key_Down);

    pushOk();
#if 0
    auto graphData = _pRegWidget->graphData();
    QCOMPARE(graphData.expression(), "${40001:32b}");
#endif
}

void TestAddRegisterWidget::registerConnection()
{
    delete _pRegWidget;

    _settingsModel.setConnectionState(Connection::ID_2, true);

    _pRegWidget = new AddRegisterWidget(&_settingsModel);

    QTest::keyClick(_pRegWidget->_pUi->cmbConnection, Qt::Key_Down);

    pushOk();
#if 0
    auto graphData = _pRegWidget->graphData();
    QCOMPARE(graphData.expression(), "${40001@2}");
#endif
}

void TestAddRegisterWidget::pushOk()
{
    //QTest::mouseClick(_pRegWidget->_pUi->buttonBox->button(QDialogButtonBox::Ok), Qt::LeftButton);
}

QTEST_MAIN(TestAddRegisterWidget)
