#include "statusbar.h"

#include <QDateTime>

#include "clickablelabel.h"
#include "guimodel.h"
#include "graphdatamodel.h"

using GuiState = GuiModel::GuiState;

const QString StatusBar::_cStateRunning = QString("Running");
const QString StatusBar::_cStateStopped = QString("Stopped");
const QString StatusBar::_cStateDataLoaded = QString("Data File loaded");
const QString StatusBar::_cStatsTemplate = QString("Success: %1\tErrors: %2");
const QString StatusBar::_cRuntime = QString("Runtime: %1");

StatusBar::StatusBar(GuiModel* pGuiModel, GraphDataModel* pGraphDataModel, QWidget *parent) :
    QStatusBar(parent), _pGuiModel(pGuiModel), _pGraphDataModel(pGraphDataModel)
{
    _pStatusState = new QLabel(_cStateStopped, this);
    _pStatusState->setFrameStyle((int)QFrame::Panel | (int)QFrame::Sunken);
    _pStatusStats = new ClickableLabel("", this);
    _pStatusStats->setFrameStyle((int)QFrame::Panel | (int)QFrame::Sunken);
    _pStatusRuntime = new QLabel("", this);
    _pStatusRuntime->setFrameStyle((int)QFrame::Panel | (int)QFrame::Sunken);

    addPermanentWidget(_pStatusState, 1);
    addPermanentWidget(_pStatusRuntime, 2);
    addPermanentWidget(_pStatusStats, 3);

    connect(_pGuiModel, &GuiModel::guiStateChanged, this, &StatusBar::updateGuiState);
    connect(_pGraphDataModel, &GraphDataModel::communicationStatsChanged, this, &StatusBar::updateStats);

    connect(_pStatusStats, &ClickableLabel::clicked, this, &StatusBar::statsClicked);
}

void StatusBar::statsClicked()
{
    emit openDiagnostics();
}

void StatusBar::updateGuiState()
{

    if (_pGuiModel->guiState() == GuiState::INIT)
    {
        _pStatusState->setText(_cStateStopped);

        _pStatusRuntime->setText(_cRuntime.arg("0:00:00"));
        _pStatusRuntime->setVisible(true);

        _pStatusStats->setText(_cStatsTemplate.arg(0).arg(0));
        _pStatusStats->setVisible(true);
    }
    else if (_pGuiModel->guiState() == GuiState::STARTED)
    {
        // Communication active
        _pStatusState->setText(_cStateRunning);

        _pStatusRuntime->setText(_cRuntime.arg("0:00:00"));
        _pStatusRuntime->setVisible(true);

        _pStatusStats->setText(_cStatsTemplate.arg(_pGraphDataModel->communicationSuccessCount()).arg(_pGraphDataModel->communicationErrorCount()));
        _pStatusStats->setVisible(true);
    }
    else if (_pGuiModel->guiState() == GuiState::STOPPED)
    {
        _pStatusState->setText(_cStateStopped);
    }
    else if (_pGuiModel->guiState() == GuiState::DATA_LOADED)
    {
        _pStatusState->setText(_cStateDataLoaded);

        _pStatusRuntime->setText(QString(""));
        _pStatusRuntime->setVisible(false);

        _pStatusStats->setText(QString(""));
        _pStatusStats->setVisible(false);
    }
}

void StatusBar::updateStats()
{
    _pStatusStats->setText(_cStatsTemplate.arg(_pGraphDataModel->communicationSuccessCount()).arg(_pGraphDataModel->communicationErrorCount()));
}

void StatusBar::updateRuntime()
{
    qint64 timePassed = QDateTime::currentMSecsSinceEpoch() - _pGraphDataModel->communicationStartTime();

    // Convert to s
    timePassed /= 1000;

    const quint32 h = static_cast<quint32>(timePassed / 3600);
    timePassed = timePassed % 3600;

    const quint32 m = static_cast<quint32>(timePassed / 60);
    timePassed = timePassed % 60;

    const quint32 s = static_cast<quint32>(timePassed);

    QString strTimePassed = QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));

    _pStatusRuntime->setText(_cRuntime.arg(strTimePassed));

}
