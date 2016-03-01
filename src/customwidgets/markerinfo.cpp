
#include "guimodel.h"
#include "graphdatamodel.h"

#include "util.h"
#include "markerinfo.h"

MarkerInfo::MarkerInfo(QWidget *parent) : QFrame(parent)
{
    _pLayout = new QVBoxLayout();

    _pGraphCombo = new QComboBox(this);
    _pGraphCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    _pTimeDataLabel = new QLabel("", this);

    _pGraphDataLabel = new QLabel("", this);

    _pLayout->addWidget(_pTimeDataLabel);
    _pLayout->addWidget(_pGraphCombo);
    _pLayout->addWidget(_pGraphDataLabel);

    _pLayout->setSpacing(1);
    _pLayout->setContentsMargins(1, 1, 1, 1); // This is redundant with setMargin, which is deprecated

    connect(_pGraphCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(graphSelected(qint32)));

    setLayout(_pLayout);
}

void MarkerInfo::setModel(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel)
{
    _pGuiModel = pGuiModel;
    _pGraphDataModel = pGraphDataModel;

    connect(_pGraphDataModel, SIGNAL(activeChanged(quint32)), this, SLOT(updateGraphList()));
    connect(_pGraphDataModel, SIGNAL(added(quint32)), this, SLOT(updateGraphList()));
    connect(_pGraphDataModel, SIGNAL(removed(quint32)), this, SLOT(removeFromGraphList(quint32)));

    connect(_pGuiModel, SIGNAL(startMarkerPosChanged()), this, SLOT(updateGraphList()));
    connect(_pGuiModel, SIGNAL(endMarkerPosChanged()), this, SLOT(updateGraphList()));

    /* TODO: Handle
     *
     * ColorChanged
     * LabelChanged
     * */

    updateGraphList();
}

void MarkerInfo::updateMarkerData()
{
    const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(_pGraphCombo->currentData().toInt());

    QCPDataMap * dataMap = _pGraphDataModel->dataMap(graphIdx);

    QString timeData = QString(
                "Start: %0\n"
                "End: %1\n"
                "Diff: %2\n"
                )
                .arg(Util::formatTime(_pGuiModel->startMarkerPos()))
                .arg(Util::formatTime(_pGuiModel->endMarkerPos()))
                .arg(Util::formatTime(_pGuiModel->endMarkerPos() - _pGuiModel->startMarkerPos()));

    /* TODO: format time difference better */
    _pTimeDataLabel->setText(timeData);

    if (graphIdx >= 0)
    {
        /*dataMap[0].value() */
        _pGraphDataLabel->setText(QString("%1").arg(graphIdx));


        QString graphData = QString(
                    "Start: %0\n"
                    "End: %1\n"
                    "Diff: %2\n"
                    )
                    .arg(dataMap->value(_pGuiModel->startMarkerPos()).value)
                    .arg(dataMap->value(_pGuiModel->endMarkerPos()).value)
                    .arg(dataMap->value(_pGuiModel->endMarkerPos()).value - dataMap->value(_pGuiModel->startMarkerPos()).value);

        /* TODO: format time difference better */
        _pGraphDataLabel->setText(graphData);
    }
    else
    {

    }
}

void MarkerInfo::updateGraphList(void)
{
    /* Get current select index */
    const qint32 currentSelectedIdx = _pGraphCombo->currentData().toInt();

    updateList();

    selectGraph(currentSelectedIdx);
}

void MarkerInfo::removeFromGraphList(const quint32 index)
{
    /* Get current select index */
    qint32 currentSelectedIdx = _pGraphCombo->currentData().toInt();

    /* correct selected index when needed */
    if (currentSelectedIdx != -1)
    {
        if ((qint32)index == currentSelectedIdx)
        {
            /* Current selected is removed */
            currentSelectedIdx = -1;
        }
        else if ((qint32)index < currentSelectedIdx)
        {
            /* Graph removed in front of our graph */
            currentSelectedIdx--;
        }
        else
        {
            /* No need to change */
        }
    }

    updateList();

    selectGraph(currentSelectedIdx);

}

void MarkerInfo::graphSelected(qint32 index)
{
    if (index > 0)
    {
         updateMarkerData();
    }
    else
    {
         _pGraphDataLabel->setText("None");
    }
}

void MarkerInfo::updateList()
{
    QList<quint16> activeGraphList;
    _pGraphDataModel->activeGraphIndexList(&activeGraphList);

    _pGraphCombo->clear();

    _pGraphCombo->addItem("None", -1);

    foreach(quint16 idx, activeGraphList)
    {
        /* TODO: add icon with color */
        _pGraphCombo->addItem(_pGraphDataModel->label(idx), QVariant(idx));
    }
}

void MarkerInfo::selectGraph(qint32 graphIndex)
{
    for(qint32 comboIdx = 0; comboIdx < _pGraphCombo->count(); comboIdx++)
    {
        if (_pGraphCombo->itemData(comboIdx).toInt() == graphIndex)
        {
            _pGraphCombo->setCurrentIndex(comboIdx);
            break;
        }
    }
}
