
#include "tst_signaltableproxymodel.h"

#include "../mocks/mockadapterhub.h"
#include "../mocks/mockdatapointmanager.h"
#include "ProtocolAdapter/datapointservice.h"
#include "dialogs/signaltableproxymodel.h"
#include "models/datapointdescriptioncache.h"
#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"

#include <QAbstractItemModelTester>
#include <QColor>
#include <QMimeData>
#include <QSignalSpy>
#include <QTest>

namespace {

void addSignal(GraphDataModel* pModel, const QString& expression)
{
    GraphData data;
    data.setLabel(QStringLiteral("Signal %1").arg(pModel->size() + 1));
    data.setExpression(expression);
    pModel->add(data);
}

} // namespace

void TestSignalTableProxyModel::init()
{
    _pGraphDataModel = new GraphDataModel(this);

    _pSettingsModel = new SettingsModel(this);
    _pSettingsModel->deviceSettings(Device::cFirstDeviceId)->setAdapterId("modbus");

    _pManager = new MockDataPointManager(QStringLiteral("modbus"), this);
    _pHub = new MockAdapterHub(this);
    _pHub->addManager(QStringLiteral("modbus"), _pManager);
    _pService = new DataPointService(_pSettingsModel, _pHub, this);
    _pCache = new DataPointDescriptionCache(_pService, this);

    _pProxy = new SignalTableProxyModel(_pGraphDataModel, _pCache, this);
}

void TestSignalTableProxyModel::cleanup()
{
    delete _pProxy;
    _pProxy = nullptr;
    delete _pCache;
    _pCache = nullptr;
    delete _pService;
    _pService = nullptr;
    delete _pHub;
    _pHub = nullptr;
    delete _pManager;
    _pManager = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
    delete _pGraphDataModel;
    _pGraphDataModel = nullptr;
}

//! Text of the Data point column of \a row for \a role.
QString TestSignalTableProxyModel::text(int row, int role) const
{
    return _pProxy->data(_pProxy->index(row, _pProxy->dataPointColumn()), role).toString();
}

void TestSignalTableProxyModel::modelTester()
{
    QAbstractItemModelTester tester(_pProxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

    addSignal(_pGraphDataModel, QStringLiteral("${40001}"));
    addSignal(_pGraphDataModel, QStringLiteral("${40002} + ${40003}"));
    addSignal(_pGraphDataModel, QStringLiteral("1 + 1"));
    _pGraphDataModel->setExpression(GraphIdx(2), QStringLiteral("${40004}"));
    _pGraphDataModel->moveSignal(GraphIdx(0), GraphIdx(2));
    _pGraphDataModel->removeSignals({ GraphIdx(1) });

    /* A description arriving changes the Data point column */
    _pProxy->data(_pProxy->index(0, _pProxy->dataPointColumn()));
    QVERIFY(!_pManager->describeCalls.isEmpty());
    _pManager->injectDescribeResult(0, QStringLiteral("Holding register 40001"));
    _pGraphDataModel->clear();

    /* Removing the last signal one by one leaves an empty model */
    addSignal(_pGraphDataModel, QStringLiteral("${40005}"));
    _pGraphDataModel->removeSignals({ GraphIdx(0) });
}

void TestSignalTableProxyModel::extraColumnAfterSourceColumns()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001}"));

    QCOMPARE(_pProxy->dataPointColumn(), static_cast<int>(GraphDataModel::column::COUNT));
    QCOMPARE(_pProxy->columnCount(), GraphDataModel::column::COUNT + 1);
    QCOMPARE(_pProxy->headerData(_pProxy->dataPointColumn(), Qt::Horizontal).toString(), QStringLiteral("Data point"));
    QCOMPARE(_pProxy->headerData(GraphDataModel::column::TEXT, Qt::Horizontal).toString(), QStringLiteral("Name"));
}

void TestSignalTableProxyModel::extraColumnIsReadOnlyAndHasNoSource()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001}"));
    const QModelIndex extra = _pProxy->index(0, _pProxy->dataPointColumn());

    QVERIFY(extra.isValid());
    QVERIFY(!(_pProxy->flags(extra) & Qt::ItemIsEditable));
    QVERIFY(_pProxy->flags(extra) & Qt::ItemIsSelectable);
    QVERIFY(_pProxy->flags(extra) & Qt::ItemIsDragEnabled);
    QVERIFY(!_pProxy->mapToSource(extra).isValid());
    QVERIFY(!_pProxy->setData(extra, QStringLiteral("x"), Qt::EditRole));
}

void TestSignalTableProxyModel::rowSelectionMapsToSourceColumnsOnly()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001}"));
    addSignal(_pGraphDataModel, QStringLiteral("${40002}"));

    const QItemSelection selection(_pProxy->index(1, 0), _pProxy->index(1, _pProxy->dataPointColumn()));
    const QItemSelection sourceSelection = _pProxy->mapSelectionToSource(selection);

    QCOMPARE(sourceSelection.size(), 1);
    QCOMPARE(sourceSelection.first().top(), 1);
    QCOMPARE(sourceSelection.first().left(), 0);
    QCOMPARE(sourceSelection.first().right(), GraphDataModel::column::COUNT - 1);

    const QItemSelection extraOnly(_pProxy->index(0, _pProxy->dataPointColumn()),
                                   _pProxy->index(0, _pProxy->dataPointColumn()));
    QVERIFY(_pProxy->mapSelectionToSource(extraOnly).isEmpty());
}

void TestSignalTableProxyModel::pendingShowsReference()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001} * 2"));

    QCOMPARE(text(0), QStringLiteral("${40001}"));
    QCOMPARE(_pManager->describeCalls, QStringList({ QStringLiteral("${40001}") }));

    /* Repainting does not ask again */
    text(0);
    QCOMPARE(_pManager->describeCalls.size(), 1);
}

void TestSignalTableProxyModel::describedShowsDescription()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001}"));
    text(0);

    _pManager->injectDescribeResult(0, QStringLiteral("Holding register 40001, unsigned 16-bit"));

    QCOMPARE(text(0), QStringLiteral("Holding register 40001, unsigned 16-bit"));
    QCOMPARE(text(0, Qt::ToolTipRole), QStringLiteral("Holding register 40001, unsigned 16-bit"));
    QVERIFY(!_pProxy->data(_pProxy->index(0, _pProxy->dataPointColumn()), Qt::ForegroundRole).isValid());
}

void TestSignalTableProxyModel::descriptionUpdatesOnlyAffectedRow()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001}"));
    addSignal(_pGraphDataModel, QStringLiteral("${40002}"));
    text(0);
    text(1);

    QSignalSpy spy(_pProxy, &SignalTableProxyModel::dataChanged);
    _pManager->injectDescribeResult(1, QStringLiteral("Holding register 40002"));

    QCOMPARE(spy.count(), 1);
    const QModelIndex topLeft = spy.at(0).at(0).value<QModelIndex>();
    const QModelIndex bottomRight = spy.at(0).at(1).value<QModelIndex>();
    QCOMPARE(topLeft.row(), 1);
    QCOMPARE(bottomRight.row(), 1);
    QCOMPARE(bottomRight.column(), _pProxy->dataPointColumn());
}

void TestSignalTableProxyModel::invalidShowsReferenceInRedWithError()
{
    addSignal(_pGraphDataModel, QStringLiteral("${99999999}"));
    text(0);

    _pManager->injectDescribeInvalid(0, QStringLiteral("Address out of range"));

    QCOMPARE(text(0), QStringLiteral("${99999999}"));
    QCOMPARE(text(0, Qt::ToolTipRole), QStringLiteral("${99999999}: Address out of range"));
    const QVariant foreground = _pProxy->data(_pProxy->index(0, _pProxy->dataPointColumn()), Qt::ForegroundRole);
    QCOMPARE(foreground.value<QColor>(), QColor(Qt::red));
}

void TestSignalTableProxyModel::invalidReferenceNamedInExpressionToolTip()
{
    addSignal(_pGraphDataModel, QStringLiteral("${99999999}"));
    text(0);
    _pManager->injectDescribeInvalid(0, QStringLiteral("Address out of range"));

    const QModelIndex expression = _pProxy->index(0, GraphDataModel::column::EXPRESSION);
    QCOMPARE(_pProxy->data(expression, Qt::ToolTipRole).toString(),
             QStringLiteral("Invalid data point: Address out of range"));
}

void TestSignalTableProxyModel::severalReferencesShowCount()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001} - ${40002}"));
    text(0);
    _pManager->injectDescribeResult(0, QStringLiteral("Holding register 40001"));

    QCOMPARE(text(0), QStringLiteral("2 data points"));
    QCOMPARE(text(0, Qt::ToolTipRole), QStringLiteral("Holding register 40001\n${40002}"));
}

void TestSignalTableProxyModel::duplicateReferencesListedTwice()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001} - ${40001}"));
    text(0);
    QCOMPARE(_pManager->describeCalls.size(), 1);
    _pManager->injectDescribeResult(0, QStringLiteral("Holding register 40001"));

    QCOMPARE(text(0), QStringLiteral("2 data points"));
    QCOMPARE(text(0, Qt::ToolTipRole), QStringLiteral("Holding register 40001\nHolding register 40001"));
}

void TestSignalTableProxyModel::syntaxErrorShowsParserError()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001} +"));
    _pGraphDataModel->setExpressionState(GraphIdx(0), GraphData::ExpressionState::SYNTAX_ERROR,
                                         QStringLiteral("Unexpected end of expression"));

    QCOMPARE(text(0), QStringLiteral("Unexpected end of expression"));
    QCOMPARE(text(0, Qt::ToolTipRole), QStringLiteral("Unexpected end of expression"));
    QVERIFY(_pManager->describeCalls.isEmpty());
}

void TestSignalTableProxyModel::unknownDeviceShownInRed()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001@5}"));
    _pGraphDataModel->setExpressionState(GraphIdx(0), GraphData::ExpressionState::UNKNOWN_DEVICE);

    QCOMPARE(text(0), QStringLiteral("Unknown device"));
    const QVariant foreground = _pProxy->data(_pProxy->index(0, _pProxy->dataPointColumn()), Qt::ForegroundRole);
    QCOMPARE(foreground.value<QColor>(), QColor(Qt::red));
    QVERIFY(_pManager->describeCalls.isEmpty());
}

void TestSignalTableProxyModel::richTextErrorShownAsPlainText()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001}"));
    _pGraphDataModel->setExpressionState(GraphIdx(0), GraphData::ExpressionState::SYNTAX_ERROR,
                                         QStringLiteral("<b>bold</b>"));

    const QString expected = QStringLiteral("<qt>&lt;b&gt;bold&lt;/b&gt;</qt>");
    QCOMPARE(text(0, Qt::ToolTipRole), expected);
    QCOMPARE(_pProxy->data(_pProxy->index(0, GraphDataModel::column::EXPRESSION), Qt::ToolTipRole).toString(),
             expected);
}

void TestSignalTableProxyModel::oneLookupPerReference()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001}"));
    text(0);
    _pManager->injectDescribeInvalid(0, QStringLiteral("Address out of range"));

    text(0);
    text(0, Qt::ToolTipRole);
    text(0, Qt::ForegroundRole);

    QCOMPARE(_pManager->describeCalls.size(), 1);
}

void TestSignalTableProxyModel::noReferenceIsEmpty()
{
    addSignal(_pGraphDataModel, QStringLiteral("1 + 1"));

    QVERIFY(text(0).isEmpty());
    QVERIFY(text(0, Qt::ToolTipRole).isEmpty());
}

void TestSignalTableProxyModel::expressionChangeUpdatesExtraColumn()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001}"));

    QSignalSpy spy(_pProxy, &SignalTableProxyModel::dataChanged);
    _pGraphDataModel->setExpression(GraphIdx(0), QStringLiteral("${40002}"));

    bool bExtraColumnChanged = false;
    for (const QList<QVariant>& arguments : std::as_const(spy))
    {
        if (arguments.at(1).value<QModelIndex>().column() == _pProxy->dataPointColumn())
        {
            bExtraColumnChanged = true;
        }
    }
    QVERIFY(bExtraColumnChanged);
    QCOMPARE(text(0), QStringLiteral("${40002}"));
}

void TestSignalTableProxyModel::mimeDataFromExtraColumnKeepsRow()
{
    addSignal(_pGraphDataModel, QStringLiteral("${40001}"));
    addSignal(_pGraphDataModel, QStringLiteral("${40002}"));

    const QModelIndexList indexes = { _pProxy->index(1, _pProxy->dataPointColumn()),
                                      _pProxy->index(1, GraphDataModel::column::TEXT) };
    QScopedPointer<QMimeData> pData(_pProxy->mimeData(indexes));
    QVERIFY(pData);
    QCOMPARE(pData->data("row"), QByteArray("1"));

    QScopedPointer<QMimeData> pExtraOnly(_pProxy->mimeData({ _pProxy->index(1, _pProxy->dataPointColumn()) }));
    QVERIFY(pExtraOnly);
    QCOMPARE(pExtraOnly->data("row"), QByteArray("1"));
}

QTEST_MAIN(TestSignalTableProxyModel)
