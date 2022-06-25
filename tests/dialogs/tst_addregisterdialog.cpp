
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
    delete _pRegDialog;
}

void TestAddRegisterDialog::registerDefault()
{
    _pRegDialog->_pUi->lineName->selectAll();
    QTest::keyClicks(_pRegDialog->_pUi->lineName, "Register 1");

    _pRegDialog->_pUi->spinAddress->selectAll();
    QTest::keyClicks(_pRegDialog->_pUi->spinAddress, "41000");

    pushOk();

    auto graphData = _pRegDialog->graphData();
    QCOMPARE(graphData.label(), "Register 1");
    QCOMPARE(graphData.expression(), "${41000}");
    QVERIFY(graphData.isActive());
}

void TestAddRegisterDialog::register32Bit()
{
#if QT_VERSION > QT_VERSION_CHECK(6, 3, 0) // Check fails inexplicably in Qt 5.15.2, Maybe related to https://bugreports.qt.io/browse/QTBUG-88143
    auto widget = _pRegDialog->_pUi->radio32Bit;
    QTest::mouseClick(widget, Qt::LeftButton, Qt::NoModifier, QPoint(widget->size().width()*3/4, widget->size().height()/2));
    pushOk();

    auto graphData = _pRegDialog->graphData();
    QCOMPARE(graphData.expression(), "${40001:32b}");
#endif
}

void TestAddRegisterDialog::registerSignedRegister()
{
    QTest::mouseClick(_pRegDialog->_pUi->radioSigned, Qt::LeftButton);
    pushOk();

    auto graphData = _pRegDialog->graphData();
    QCOMPARE(graphData.expression(), "${40001:s16b}");
}

void TestAddRegisterDialog::registerConnection()
{
    delete _pRegDialog;

    _settingsModel.setConnectionState(Connection::ID_2, true);

    _pRegDialog = new AddRegisterDialog(&_settingsModel);

    QTest::keyClick(_pRegDialog->_pUi->cmbConnection, Qt::Key_Down);

    pushOk();

    auto graphData = _pRegDialog->graphData();
    QCOMPARE(graphData.expression(), "${40001@2}");
}

void TestAddRegisterDialog::pushOk()
{
    QTest::mouseClick(_pRegDialog->_pUi->buttonBox->button(QDialogButtonBox::Ok), Qt::LeftButton);
}

QTEST_MAIN(TestAddRegisterDialog)
