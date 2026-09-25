#include "tst_graphview.h"

#include "graphview/graphview.h"
#include "graphview/scopeplot.h"
#include "models/communicationstatsmodel.h"
#include "models/graphdatamodel.h"
#include "models/guimodel.h"
#include "models/notemodel.h"
#include "models/settingsmodel.h"
#include "util/graphindex.h"

#include <QTest>
#include <QWidget>

static DataQuality::Quality quality(DataQuality::State state, DataQuality::Flags flags = DataQuality::Flag::NoFlags)
{
    return DataQuality::Quality{ state, flags };
}

void TestGraphView::init()
{
    _pHost = new QWidget();
    _pPlot = new ScopePlot(_pHost);

    _pGuiModel = new GuiModel(this);
    _pSettingsModel = new SettingsModel(this);
    _pGraphDataModel = new GraphDataModel(this);
    _pCommunicationStatsModel = new CommunicationStatsModel(this);
    _pNoteModel = new NoteModel(this);

    _pGraphView = new GraphView(_pGuiModel, _pSettingsModel, _pGraphDataModel, _pCommunicationStatsModel, _pNoteModel,
                                _pPlot, this);
}

void TestGraphView::cleanup()
{
    delete _pGraphView;
    _pGraphView = nullptr;

    delete _pNoteModel;
    _pNoteModel = nullptr;
    delete _pCommunicationStatsModel;
    _pCommunicationStatsModel = nullptr;
    delete _pGraphDataModel;
    _pGraphDataModel = nullptr;
    delete _pSettingsModel;
    _pSettingsModel = nullptr;
    delete _pGuiModel;
    _pGuiModel = nullptr;

    delete _pHost;
    _pHost = nullptr;
    _pPlot = nullptr;
}

/*!
 * Reproduces a bug where clearGraph(), on its "keep timestamps, zero values" path (taken whenever
 * more than one graph is active), zeroed a sample's value but left its data quality untouched.
 * A sample that used to be Invalid/Degraded therefore kept reporting that stale quality after the
 * graph was cleared, even though its value is now a meaningless placeholder rather than a real
 * reading. The fix resets quality to NoValue alongside the value, matching the padding samples
 * GraphView::updateGraphs() already creates for the same "no measurement for this timestamp" case.
 */
void TestGraphView::clearGraphWithMultipleActiveGraphsResetsQualityToNoValue()
{
    _pGraphDataModel->add();
    _pGraphDataModel->add();
    _pGraphView->updateGraphs();

    const GraphIdx graphIdx(0);
    _pGraphDataModel->mutableDataSeries(graphIdx)->add(100.0, 42.0, quality(DataQuality::State::Invalid));

    _pGraphView->clearGraph(graphIdx);

    const QSharedPointer<const GraphDataSeries> pSeries = _pGraphDataModel->dataSeries(graphIdx);
    QCOMPARE(pSeries->size(), 1);

    const GraphSample& sample = *pSeries->constBegin();
    QCOMPARE(sample.value, 0.0);
    QCOMPARE(sample.quality.state, DataQuality::State::NoValue);
}

void TestGraphView::qualityMarkersBucketSamplesByState()
{
    _pGraphDataModel->add();

    const GraphIdx graphIdx(0);
    QSharedPointer<GraphDataSeries> pSeries = _pGraphDataModel->mutableDataSeries(graphIdx);
    pSeries->add(1.0, 10.0, quality(DataQuality::State::Good));
    pSeries->add(2.0, 20.0, quality(DataQuality::State::Degraded, DataQuality::Flag::Overflow));
    pSeries->add(3.0, 30.0, quality(DataQuality::State::Invalid));
    pSeries->add(4.0, 40.0, quality(DataQuality::State::NoValue));
    pSeries->add(5.0, 50.0, quality(DataQuality::State::Invalid));

    _pGraphView->updateGraphs();

    /* The overlays must not be QCPGraphs: the whole plot code addresses graph(i) positionally
       and assumes exactly one graph per active signal. */
    QCOMPARE(_pPlot->graphCount(), 1);
    QCOMPARE(markerCurves().size(), 3);

    QCOMPARE(markerCount(QCPScatterStyle::ssTriangle), 1);
    QCOMPARE(markerCount(QCPScatterStyle::ssCustom), 2);
    QCOMPARE(markerCount(QCPScatterStyle::ssSquare), 1);

    /* Markers carry the coordinates of the sample they mark */
    const QCPCurve* pInvalidCurve = markerCurve(QCPScatterStyle::ssCustom);
    QVERIFY(pInvalidCurve != nullptr);
    QCOMPARE(pInvalidCurve->data()->constBegin()->key, 3.0);
    QCOMPARE(pInvalidCurve->data()->constBegin()->value, 30.0);
}

void TestGraphView::qualityMarkersShowPaddedSamplesAsNoValue()
{
    _pGraphDataModel->add();
    _pGraphDataModel->add();

    _pGraphDataModel->mutableDataSeries(GraphIdx(0))->add(1.0, 10.0, quality(DataQuality::State::Good));
    _pGraphDataModel->mutableDataSeries(GraphIdx(0))->add(2.0, 20.0, quality(DataQuality::State::Good));

    /* The second graph has no samples, so updateGraphs() pads it to match the first one */
    _pGraphView->updateGraphs();

    QCOMPARE(markerCurves().size(), 6);
    QCOMPARE(markerCount(QCPScatterStyle::ssSquare), 2);
}

void TestGraphView::qualityMarkersAppendLiveSample()
{
    _pGraphDataModel->add();
    _pGraphView->updateGraphs();

    QCOMPARE(markerCount(QCPScatterStyle::ssCustom), 0);

    ResultDoubleList resultList;
    resultList.append(ResultDouble(0.0, DataQuality::State::Invalid));
    _pGraphView->plotResults(resultList);

    QCOMPARE(markerCount(QCPScatterStyle::ssCustom), 1);
    QCOMPARE(markerCount(QCPScatterStyle::ssTriangle), 0);
}

void TestGraphView::qualityMarkersFollowGraphVisibility()
{
    _pGraphDataModel->add();
    _pGraphDataModel->mutableDataSeries(GraphIdx(0))->add(1.0, 10.0, quality(DataQuality::State::Invalid));
    _pGraphView->updateGraphs();

    QVERIFY(markerCurve(QCPScatterStyle::ssCustom) != nullptr);
    QVERIFY(markerCurve(QCPScatterStyle::ssCustom)->visible());

    _pGraphDataModel->setVisible(GraphIdx(0), false);
    _pGraphView->handleGraphVisibilityChange(GraphIdx(0));

    QVERIFY(!markerCurve(QCPScatterStyle::ssCustom)->visible());
}

QList<QCPCurve*> TestGraphView::markerCurves() const
{
    QList<QCPCurve*> curveList;

    for (int idx = 0; idx < _pPlot->plottableCount(); idx++)
    {
        QCPCurve* pCurve = qobject_cast<QCPCurve*>(_pPlot->plottable(idx));
        if (pCurve != nullptr)
        {
            curveList.append(pCurve);
        }
    }

    return curveList;
}

QCPCurve* TestGraphView::markerCurve(QCPScatterStyle::ScatterShape shape) const
{
    const QList<QCPCurve*> curveList = markerCurves();
    for (QCPCurve* pCurve : curveList)
    {
        if (pCurve->scatterStyle().shape() == shape)
        {
            return pCurve;
        }
    }

    return nullptr;
}

int TestGraphView::markerCount(QCPScatterStyle::ScatterShape shape) const
{
    int count = 0;

    const QList<QCPCurve*> curveList = markerCurves();
    for (const QCPCurve* pCurve : curveList)
    {
        if (pCurve->scatterStyle().shape() == shape)
        {
            count += pCurve->data()->size();
        }
    }

    return count;
}

QTEST_MAIN(TestGraphView)
