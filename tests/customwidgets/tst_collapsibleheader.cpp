#include "tst_collapsibleheader.h"

#include "customwidgets/collapsibleheader.h"

#include <QLayout>
#include <QSignalSpy>
#include <QTest>
#include <QToolButton>

void TestCollapsibleHeader::init()
{
    _pParent = new QWidget();
    _pHeader = new CollapsibleHeader(_pParent);
    _pContent = new QWidget(_pParent);
    _pHeader->setText(QStringLiteral("&Section"));
    _pHeader->setContent(_pContent);
}

void TestCollapsibleHeader::cleanup()
{
    delete _pParent;
    _pParent = nullptr;
    _pHeader = nullptr;
    _pContent = nullptr;
}

//! A new section is collapsed: content hidden, arrow pointing right.
void TestCollapsibleHeader::startsCollapsed()
{
    QVERIFY(!_pHeader->isExpanded());
    QVERIFY(_pContent->isHidden());
    QCOMPARE(_pHeader->toggleButton()->arrowType(), Qt::RightArrow);
}

//! Expanding shows the content and turns the arrow down; collapsing reverts both.
void TestCollapsibleHeader::expandShowsContentAndTurnsArrow()
{
    QSignalSpy spy(_pHeader, &CollapsibleHeader::expandedChanged);

    _pHeader->setExpanded(true);
    QVERIFY(!_pContent->isHidden());
    QCOMPARE(_pHeader->toggleButton()->arrowType(), Qt::DownArrow);

    _pHeader->setExpanded(false);
    QVERIFY(_pContent->isHidden());
    QCOMPARE(_pHeader->toggleButton()->arrowType(), Qt::RightArrow);

    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(0).at(0).toBool(), true);
    QCOMPARE(spy.at(1).at(0).toBool(), false);
}

//! Content set on an expanded section is shown right away.
void TestCollapsibleHeader::setContentAppliesCurrentState()
{
    _pHeader->setExpanded(true);

    auto* pOther = new QWidget(_pParent);
    pOther->setVisible(false);
    _pHeader->setContent(pOther);

    QVERIFY(!pOther->isHidden());
}

//! Clicking the arrow button expands the section.
void TestCollapsibleHeader::clickOnToggleExpands()
{
    QTest::mouseClick(_pHeader->toggleButton(), Qt::LeftButton);

    QVERIFY(_pHeader->isExpanded());
    QVERIFY(!_pContent->isHidden());
}

//! A click on the row next to the title also toggles the section.
void TestCollapsibleHeader::clickOnRowTogglesSection()
{
    _pHeader->resize(400, _pHeader->sizeHint().height());
    _pHeader->layout()->setGeometry(_pHeader->rect());
    const QPoint rowPoint(_pHeader->width() - 5, _pHeader->height() / 2);
    QVERIFY(!_pHeader->toggleButton()->geometry().contains(rowPoint));

    QTest::mousePress(_pHeader, Qt::LeftButton, Qt::NoModifier, rowPoint);
    QVERIFY(_pHeader->isExpanded());

    QTest::mousePress(_pHeader, Qt::LeftButton, Qt::NoModifier, rowPoint);
    QVERIFY(!_pHeader->isExpanded());
}

QTEST_MAIN(TestCollapsibleHeader)
