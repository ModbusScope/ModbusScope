#include "tst_graphmenucontroller.h"

#include "customwidgets/overlaylabel.h"
#include "dialogs/graphmenucontroller.h"
#include "models/graphdatamodel.h"
#include "util/graphindex.h"

#include <QAction>
#include <QMenu>
#include <QSignalSpy>
#include <QTest>
#include <QWidget>

void TestGraphMenuController::init()
{
    _pGraphDataModel = new GraphDataModel(this);
    _pHost = new QWidget();
    _pMenu = new QMenu(_pHost);
    _pOverlayLabel = new OverlayLabel(QStringLiteral("test"), _pHost);

    _pController = new GraphMenuController(_pGraphDataModel, _pMenu, _pOverlayLabel, this);
    _pController->handleGraphsCountChanged();
}

void TestGraphMenuController::cleanup()
{
    delete _pController;
    _pController = nullptr;

    delete _pHost;
    _pHost = nullptr;
    _pMenu = nullptr;
    _pOverlayLabel = nullptr;

    delete _pGraphDataModel;
    _pGraphDataModel = nullptr;
}

void TestGraphMenuController::emptyModelDisablesMenu()
{
    QSignalSpy spy(_pController, &GraphMenuController::activeGraphsChanged);

    _pController->handleGraphsCountChanged();

    QVERIFY(!_pMenu->isEnabled());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toBool(), false);
}

void TestGraphMenuController::addGraphsBuildsMenu()
{
    QSignalSpy spy(_pController, &GraphMenuController::activeGraphsChanged);

    _pGraphDataModel->add();
    _pGraphDataModel->setLabel(GraphIdx(0), QStringLiteral("First"));
    _pGraphDataModel->add();
    _pGraphDataModel->setLabel(GraphIdx(1), QStringLiteral("Second"));

    QCOMPARE(_pMenu->actions().size(), 2);
    QVERIFY(_pMenu->isEnabled());
    QCOMPARE(_pMenu->actions().at(0)->text(), QStringLiteral("First"));
    QCOMPARE(_pMenu->actions().at(1)->text(), QStringLiteral("Second"));
    QVERIFY(_pMenu->actions().at(0)->isCheckable());
    QVERIFY(_pMenu->actions().at(0)->isChecked());

    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy.last().at(0).toBool(), true);
}

void TestGraphMenuController::labelChangeUpdatesAction()
{
    _pGraphDataModel->add();

    _pGraphDataModel->setLabel(GraphIdx(0), QStringLiteral("Renamed"));

    QCOMPARE(_pMenu->actions().at(0)->text(), QStringLiteral("Renamed"));
}

void TestGraphMenuController::colorChangeUpdatesIcon()
{
    _pGraphDataModel->add();

    _pGraphDataModel->setColor(GraphIdx(0), QColor(Qt::red));

    QVERIFY(!_pMenu->actions().at(0)->icon().isNull());
}

void TestGraphMenuController::visibilityChangeChecksAction()
{
    _pGraphDataModel->add();
    QVERIFY(_pMenu->actions().at(0)->isChecked());

    _pGraphDataModel->setVisible(GraphIdx(0), false);

    QVERIFY(!_pMenu->actions().at(0)->isChecked());
}

void TestGraphMenuController::menuClickTogglesModelVisibility()
{
    _pGraphDataModel->add();
    QVERIFY(_pGraphDataModel->isVisible(GraphIdx(0)));

    _pMenu->actions().at(0)->setChecked(false);

    QVERIFY(!_pGraphDataModel->isVisible(GraphIdx(0)));
}

void TestGraphMenuController::inactiveGraphIgnored()
{
    _pGraphDataModel->add();
    _pGraphDataModel->add();
    QCOMPARE(_pMenu->actions().size(), 2);

    _pGraphDataModel->setActive(GraphIdx(0), false);

    QCOMPARE(_pMenu->actions().size(), 1);

    // Changing a now-inactive graph must not crash or affect the remaining menu entry
    _pGraphDataModel->setLabel(GraphIdx(0), QStringLiteral("Ignored"));
    _pGraphDataModel->setColor(GraphIdx(0), QColor(Qt::blue));
    _pGraphDataModel->setVisible(GraphIdx(0), false);

    QCOMPARE(_pMenu->actions().size(), 1);
}

QTEST_MAIN(TestGraphMenuController)
