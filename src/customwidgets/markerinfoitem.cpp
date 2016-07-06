
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

    if (graphIdx >= 0)
    {
        QCPDataMap * dataMap = _pGraphDataModel->dataMap(graphIdx);
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
    double result = 0;
    const qint32 graphIdx = _pGraphCombo->currentData().toInt();

    if (graphIdx >= 0)
    {

        QCPDataMap * pDataMap = _pGraphDataModel->dataMap(graphIdx);
        const double valueDiff = pDataMap->value(_pGuiModel->endMarkerPos()).value - pDataMap->value(_pGuiModel->startMarkerPos()).value;
        const double timeDiff = _pGuiModel->endMarkerPos() - _pGuiModel->startMarkerPos();

        QCPDataMap::iterator dataPoint;
        QCPDataMap::iterator start;
        QCPDataMap::iterator end;

        /* make sure we go in ascending order */
        if (_pGuiModel->endMarkerPos() > _pGuiModel->startMarkerPos())
        {
            start = pDataMap->lowerBound(_pGuiModel->startMarkerPos());
            end = pDataMap->upperBound(_pGuiModel->endMarkerPos());
        }
        else
        {
            /* Change order */
            start = pDataMap->lowerBound(_pGuiModel->endMarkerPos());
            end = pDataMap->upperBound(_pGuiModel->startMarkerPos());
        }


        if (expressionMask == GuiModel::cDifferenceMask)
        {
            result = valueDiff;
        }
        else if (expressionMask == GuiModel::cSlopeMask)
        {
            result = valueDiff / timeDiff;
        }
        else if (expressionMask == GuiModel::cAverageMask)
        {
            double avg = 0;
            quint32 count = 0;
            for (dataPoint = start; dataPoint != end; dataPoint++)
            {
                count++;
                avg += dataPoint.value().value;
            }

            if (count == 0)
            {
                result = 0;
            }
            else
            {
                result = avg / count;
            }
        }
        else if (expressionMask == GuiModel::cMinimumMask)
        {
            double min = 0xFFFFFFFF;

            for (dataPoint = start; dataPoint != end; dataPoint++)
            {
                if (dataPoint.value().value < min)
                {
                    min = dataPoint.value().value;
                }
            }

            result = min;
        }
        else if (expressionMask == GuiModel::cMaximumMask)
        {
            double max = -1 * 0xFFFFFFFF;;

            for (dataPoint = start; dataPoint != end; dataPoint++)
            {
                if (dataPoint.value().value > max)
                {
                    max = dataPoint.value().value;
                }
            }

            result = max;
        }
        else if (expressionMask == GuiModel::cCustomMask)
        {
            /* TODO: call python script */
        }
        else
        {
            result = 0;
        }
    }

    return result;
}
