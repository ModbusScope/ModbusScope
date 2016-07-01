
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

    QWidget * pInfoWidget = new QWidget(this);
    QHBoxLayout *pInfoLayout = new QHBoxLayout(this);

    _pGraphDataLabelLeft = new QLabel("", this);
    _pGraphDataLabelRight = new QLabel("", this);
    _pGraphDataLabelRight->setAlignment(Qt::AlignTop);

    pInfoLayout->setSpacing(0);
    pInfoLayout->setContentsMargins(0, 2, 0, 0); // This is redundant with setMargin, which is deprecated
    pInfoLayout->addWidget(_pGraphDataLabelLeft);
    pInfoLayout->addWidget(_pGraphDataLabelRight);

    pInfoWidget->setLayout(pInfoLayout);

    _pLayout->addWidget(_pGraphCombo);
    _pLayout->addWidget(pInfoWidget);

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

    connect(_pGuiModel, SIGNAL(markerExpressionMaskChanged()), this, SLOT(updateData()));
    connect(_pGuiModel, SIGNAL(markerExpressionCustomScriptChanged()), this, SLOT(updateData()));

    updateGraphList();
}

void MarkerInfoItem::updateData()
{
    const qint32 graphIdx = _pGraphCombo->currentData().toInt();

    QCPDataMap * dataMap = _pGraphDataModel->dataMap(graphIdx);

    if (graphIdx >= 0)
    {
        QStringList expressionList;
        const quint32 mask = _pGuiModel->markerExpressionMask();

        for(qint32 idx = 0; idx < GuiModel::cMarkerExpressionBits.size(); idx++)
        {
            if (mask & GuiModel::cMarkerExpressionBits[idx])
            {
                QString expression = GuiModel::cMarkerExpressionStrings[idx];
                const double expressionValue = calculateMarkerExpressionValue(GuiModel::cMarkerExpressionBits[idx]);

                expressionList.append(expression.arg(Util::formatDoubleForExport(expressionValue)));
            }
        }

        /* Add permanent items (y1, y2) */
        expressionList.prepend(GuiModel::cMarkerExpressionEnd.arg(Util::formatDoubleForExport(dataMap->value(_pGuiModel->endMarkerPos()).value)));
        expressionList.prepend(GuiModel::cMarkerExpressionStart.arg(Util::formatDoubleForExport(dataMap->value(_pGuiModel->startMarkerPos()).value)));

        /* Construct labels data */
        const qint32 leftRowCount = expressionList.size() - expressionList.size() / 2;
        QString graphDataLeft;
        QString graphDataRight;
        for(qint32 idx = 0; idx < expressionList.size(); idx++)
        {
            if (idx < leftRowCount)
            {
                graphDataLeft.append(expressionList[idx]);
            }
            else
            {
                graphDataRight.append(expressionList[idx]);
            }
        }

        _pGraphDataLabelLeft->setText(graphDataLeft);
        _pGraphDataLabelRight->setText(graphDataRight);


#if 0
        const double valueDiff = dataMap->value(_pGuiModel->endMarkerPos()).value - dataMap->value(_pGuiModel->startMarkerPos()).value;
        const double timeDiff = _pGuiModel->endMarkerPos() - _pGuiModel->startMarkerPos();

        QString graphDataLeft = QString(
                    "y1: %0\n"
                    "y2: %1\n"
                    "Diff: %2"
                    )
                    .arg(dataMap->value(_pGuiModel->startMarkerPos()).value)
                    .arg(dataMap->value(_pGuiModel->endMarkerPos()).value)
                    .arg(valueDiff);

        _pGraphDataLabelLeft->setText(graphDataLeft);

        QString graphDataRight = QString(
                    "Slope: %0\n"
                    "\n"
                    ""
                    )
                    .arg(Util::formatDoubleForExport(valueDiff / (timeDiff / 1000)));
        _pGraphDataLabelRight->setText(graphDataRight);
#endif
    }
    else
    {
        /* No graph selected */
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
         _pGraphDataLabelLeft->setText("None");
         _pGraphDataLabelRight->setText("");
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

double MarkerInfoItem::calculateMarkerExpressionValue(quint32 expressionMask)
{
    //TODO: calculate data over marker range
    return 0;
}
