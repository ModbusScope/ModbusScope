
#include "tst_addregisterwidget.h"

#include "dialogs/addregisterwidget.h"
#include "ui_addregisterwidget.h"

#include <QSignalSpy>
#include <QTest>

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
    QTest::keyClicks(_pRegWidget->_pUi->spinAddress, "100");

    GraphData graphData;
    addRegister(graphData);

    QCOMPARE(graphData.label(), "Register 1");
    QCOMPARE(graphData.expression(), "${40101}");
    QVERIFY(graphData.isActive());
}

void TestAddRegisterWidget::registerType()
{
    QTest::keyClick(_pRegWidget->_pUi->cmbType, Qt::Key_Down);

    GraphData graphData;
    addRegister(graphData);

    QCOMPARE(graphData.expression(), "${40001:32b}");
}

void TestAddRegisterWidget::registerObjectType()
{
    QTest::keyClick(_pRegWidget->_pUi->cmbObjectType, Qt::Key_Up);

    GraphData graphData;
    addRegister(graphData);

    QCOMPARE(graphData.expression(), "${30001}");
}

void TestAddRegisterWidget::registerConnection()
{
    delete _pRegWidget;

    _settingsModel.setConnectionState(ConnectionId::ID_2, true);

    _pRegWidget = new AddRegisterWidget(&_settingsModel);

    QTest::keyClick(_pRegWidget->_pUi->cmbConnection, Qt::Key_Down);

    GraphData graphData;
    addRegister(graphData);

    QCOMPARE(graphData.expression(), "${40001@2}");
}

void TestAddRegisterWidget::registerValueAxis()
{
    QTest::mouseClick(_pRegWidget->_pUi->radioSecondary, Qt::LeftButton);

    GraphData graphData;
    addRegister(graphData);

    QCOMPARE(graphData.valueAxis(), GraphData::VALUE_AXIS_SECONDARY);
}

void TestAddRegisterWidget::pushOk()
{
    QTest::mouseClick(_pRegWidget->_pUi->btnAdd, Qt::LeftButton);
}

void TestAddRegisterWidget::addRegister(GraphData &graphData)
{
    QSignalSpy spyGraphDataConfigured(_pRegWidget, &AddRegisterWidget::graphDataConfigured);

    pushOk();

    QCOMPARE(spyGraphDataConfigured.count(), 1);

    QList<QVariant> arguments = spyGraphDataConfigured.takeFirst();
    QCOMPARE(arguments.count(), 1);

    QVERIFY(arguments.first().canConvert<GraphData>());

    graphData = arguments.first().value<GraphData>();
}

QTEST_MAIN(TestAddRegisterWidget)
