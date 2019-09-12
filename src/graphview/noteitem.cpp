#include "noteitem.h"


NoteItem::NoteItem(MyQCustomPlot* pPlot, const QString& rText, const QPointF& rPosition)
    : QCPItemText(pPlot),
      _arrow(pPlot)
{  
    setPositionAlignment(Qt::AlignTop|Qt::AlignLeft);
    setTextAlignment(Qt::AlignLeft);
    setText(rText);
    position->setType(QCPItemPosition::ptPlotCoords);
    setPen(QPen(Qt::black)); // show black border around text
    position->setCoords(rPosition); // place position at left/top of axis rect

    _arrow.start->setParentAnchor(bottom);
    _arrow.end->setCoords(rPosition);
    _arrow.setHead(QCPLineEnding::esSpikeArrow);
}

NoteItem::~NoteItem()
{

}

void NoteItem::setArrowPosition(const QPointF& rPosition)
{
    _arrow.end->setCoords(rPosition);
}
