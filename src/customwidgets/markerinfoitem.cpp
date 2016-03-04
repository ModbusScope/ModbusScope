
#include "guimodel.h"
#include "graphdatamodel.h"

#include "util.h"
#include "markerinfoitem.h"

MarkerInfoItem::MarkerInfoItem(QWidget *parent) : QFrame(parent)
{
    _pLayout = new QVBoxLayout();

    _pLayout->setSpacing(0);
    _pLayout->setContentsMargins(0, 0, 0, 0); // This is redundant with setMargin, which is deprecated

    _pGraphCombo = new QComboBox(this);
    _pGraphCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    _pGraphDataLabel = new QLabel("", this);

    _pLayout->addWidget(_pGraphCombo);
    _pLayout->addWidget(_pGraphDataLabel);

    connect(_pGraphCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(graphSelected(qint32)));

    setLayout(_pLayout);
}

void MarkerInfoItem::setModel(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel)
{
    _pGuiModel = pGuiModel;
    _pGraphDataModel = pGraphDataModel;

    connect(_pGraphDataModel, SIGNAL(activeChanged(quint32)), this, SLOT(updateGraphList()));
    connect(_pGraphDataModel, SIGNAL(added(quint32)), this, SLOT(updateGraphList()));
    connect(_pGraphDataModel, SIGNAL(removed(quint32)), this, SLOT(removeFromGraphList(quint32)));
    connect(_pGraphDataModel, SIGNAL(colorChanged(quint32)), this, SLOT(updateColor(quint32)));
    connect(_pGraphDataModel, SIGNAL(labelChanged(quint32)), this, SLOT(updateLabel(quint32)));

    connect(_pGuiModel, SIGNAL(startMarkerPosChanged()), this, SLOT(updateGraphList()));
    connect(_pGuiModel, SIGNAL(endMarkerPosChanged()), this, SLOT(updateGraphList()));

    updateGraphList();
}

void MarkerInfoItem::updateData()
{
    const qint32 graphIdx = _pGraphCombo->currentData().toInt();

    QCPDataMap * dataMap = _pGraphDataModel->dataMap(graphIdx);

    if (graphIdx >= 0)
    {
        /*dataMap[0].value() */
        _pGraphDataLabel->setText(QString("%1").arg(graphIdx));


        QString graphData = QString(
                    "y1: %0\n"
                    "y2: %1\n"
                    "Diff: %2\n"
                    )
                    .arg(dataMap->value(_pGuiModel->startMarkerPos()).value)
                    .arg(dataMap->value(_pGuiModel->endMarkerPos()).value)
                    .arg(dataMap->value(_pGuiModel->endMarkerPos()).value - dataMap->value(_pGuiModel->startMarkerPos()).value);

        _pGraphDataLabel->setText(graphData);
    }
    else
    {

    }
}

void MarkerInfoItem::updateGraphList(void)
{
    /* Get current select index */
    const qint32 currentSelectedIdx = _pGraphCombo->currentData().toInt();

    updateList();

    selectGraph(currentSelectedIdx);
}

void MarkerInfoItem::updateColor(quint32 graphIdx)
{
    QPixmap pixmap(20,5);
    pixmap.fill(_pGraphDataModel->color(graphIdx));

    QIcon graphIcon = QIcon(pixmap);

    /* + 1 for none selection */
    _pGraphCombo->setItemIcon(_pGraphDataModel->convertToActiveGraphIndex(graphIdx) + 1, graphIcon);
}

void MarkerInfoItem::updateLabel(quint32 graphIdx)
{
    _pGraphCombo->setItemText(_pGraphDataModel->convertToActiveGraphIndex(graphIdx) + 1, _pGraphDataModel->label(graphIdx));
}

void MarkerInfoItem::removeFromGraphList(const quint32 index)
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

void MarkerInfoItem::graphSelected(qint32 index)
{
    if (index > 0)
    {
         updateData();
    }
    else
    {
         _pGraphDataLabel->setText("None");
    }
}

void MarkerInfoItem::updateList()
{
    QList<quint16> activeGraphList;
    _pGraphDataModel->activeGraphIndexList(&activeGraphList);

    _pGraphCombo->clear();

    _pGraphCombo->addItem("None", -1);

    foreach(quint16 idx, activeGraphList)
    {
        _pGraphCombo->addItem(_pGraphDataModel->label(idx), QVariant(idx));

        updateColor(idx);
    }
}

void MarkerInfoItem::selectGraph(qint32 graphIndex)
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
