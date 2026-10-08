#include "tst_quickstartdialog.h"

#include "dialogs/quickstartdialog.h"

#include <QPushButton>
#include <QSignalSpy>
#include <QTest>

void TestQuickStartDialog::dontShowAgainDefaultsToFalse()
{
    QuickStartDialog dialog;

    QVERIFY(!dialog.dontShowAgain());
}

void TestQuickStartDialog::setDontShowAgainRoundTrip()
{
    QuickStartDialog dialog;

    dialog.setDontShowAgain(true);
    QVERIFY(dialog.dontShowAgain());

    dialog.setDontShowAgain(false);
    QVERIFY(!dialog.dontShowAgain());
}

void TestQuickStartDialog::showDocsButtonEmitsSignal()
{
    QuickStartDialog dialog;
    dialog.show();
    QSignalSpy spy(&dialog, &QuickStartDialog::showDocsRequested);

    auto* pButton = dialog.findChild<QPushButton*>("btnShowDocs");
    QVERIFY(pButton != nullptr);
    QTest::mouseClick(pButton, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    QVERIFY(dialog.isVisible());
}

void TestQuickStartDialog::closeButtonRejects()
{
    QuickStartDialog dialog;
    dialog.setDontShowAgain(true);
    dialog.show();

    auto* pButton = dialog.findChild<QPushButton*>("btnClose");
    QVERIFY(pButton != nullptr);
    QTest::mouseClick(pButton, Qt::LeftButton);

    QCOMPARE(dialog.result(), static_cast<int>(QDialog::Rejected));
    QVERIFY(!dialog.isVisible());
    QVERIFY(dialog.dontShowAgain());
}

QTEST_MAIN(TestQuickStartDialog)
