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
    _pGraphDataModel->mutableDataSeries(graphIdx)->add(
      100.0, 42.0, DataQuality::Quality{ DataQuality::State::Invalid, DataQuality::Flag::NoFlags });

    _pGraphView->clearGraph(graphIdx);

    const QSharedPointer<const GraphDataSeries> pSeries = _pGraphDataModel->dataSeries(graphIdx);
    QCOMPARE(pSeries->size(), 1);

    const GraphSample& sample = *pSeries->constBegin();
    QCOMPARE(sample.value, 0.0);
    QCOMPARE(sample.quality.state, DataQuality::State::NoValue);
}

QTEST_MAIN(TestGraphView)
