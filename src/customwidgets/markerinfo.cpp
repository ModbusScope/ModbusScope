
#include "guimodel.h"
#include "graphdatamodel.h"

#include "formatrelativetime.h"
#include "markerinfoitem.h"
#include "markerinfo.h"
#include "markerinfodialog.h"

MarkerInfo::MarkerInfo(QWidget *parent) : QFrame(parent)
{
    _pLayout = new QVBoxLayout();

    setContentsMargins(2, 2, 2, 2); // This is redundant with setMargin, which is deprecated

    _pTimeDataLabel = new QLabel("", this);
    _pLayout->addWidget(_pTimeDataLabel);
    
    for (quint32 idx = 0; idx < graphMarkerCount; idx++)
    {
        MarkerInfoItem * pGraphMarkerData = new MarkerInfoItem();

        graphMarkerData.append(pGraphMarkerData);
        _pLayout->addWidget(pGraphMarkerData);
    }

    setLayout(_pLayout);

    // For rightclick menu
    _pEditMarkerInfoMenu = new QMenu(parent);
    _pEditMarkerInfoAction = _pEditMarkerInfoMenu->addAction("Edit...");
    connect(_pEditMarkerInfoAction, &QAction::triggered, this, &MarkerInfo::showMarkerInfoDialog);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &MarkerInfo::customContextMenuRequested, this, &MarkerInfo::showContextMenu);

}

MarkerInfo::~MarkerInfo()
{
    delete _pEditMarkerInfoMenu;
}

void MarkerInfo::setModel(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel)
{
    _pGuiModel = pGuiModel;
    _pGraphDataModel = pGraphDataModel;

    connect(_pGuiModel, &GuiModel::startMarkerPosChanged, this, &MarkerInfo::updateMarkerData);
    connect(_pGuiModel, &GuiModel::endMarkerPosChanged, this, &MarkerInfo::updateMarkerData);

    updateMarkerData();

    for (quint32 idx = 0; idx < graphMarkerCount; idx++)
    {
        graphMarkerData[idx]->setModel(_pGuiModel, _pGraphDataModel);
    }
}

void MarkerInfo::updateMarkerData()
{
    QString timeData = QString(
                "<b>Time 1</b>: %0<br />"
                "<b>Time 2</b>: %1<br />"
                "<b>Time Diff</b>: %2<br />"
                )
                .arg(FormatRelativeTime::formatTime(_pGuiModel->startMarkerPos()))
                .arg(FormatRelativeTime::formatTime(_pGuiModel->endMarkerPos()))
                .arg(FormatRelativeTime::formatTimeDiff(_pGuiModel->endMarkerPos() - _pGuiModel->startMarkerPos()));

    _pTimeDataLabel->setText(timeData);
}

void MarkerInfo::showContextMenu(const QPoint& pos)
{
    _pEditMarkerInfoMenu->popup(mapToGlobal(pos));
}

void MarkerInfo::showMarkerInfoDialog()
{
    MarkerInfoDialog *pMarkerInfoDialog = new MarkerInfoDialog(_pGuiModel, this);

    pMarkerInfoDialog->exec();
}


