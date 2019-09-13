#include "noteitem.h"

NoteItem::NoteItem(MyQCustomPlot* pPlot,
                   const QString& rText,
                   const QPointF& rPosition,
                   const QPointF& rArrowPosition)
    : QCPItemText(pPlot),
      _arrow(pPlot)
{  
    setPositionAlignment(Qt::AlignTop|Qt::AlignLeft);
    setTextAlignment(Qt::AlignLeft);
    setText(rText);
    position->setType(QCPItemPosition::ptPlotCoords);
    setPen(QPen(Qt::black)); // show black border around text
    position->setCoords(rPosition); // place position at left/top of axis rect

    _arrow.setHead(QCPLineEnding::esSpikeArrow);
    setArrowPosition(rArrowPosition);
}

NoteItem::~NoteItem()
{
    parentPlot()->removeItem(&_arrow);
}

void NoteItem::setArrowPosition(const QPointF& rPosition)
{
    _arrow.end->setCoords(rPosition);
    updateArrowAnchor();
}

void NoteItem::setNotePosition(const QPointF& rPosition)
{
    position->setCoords(rPosition); // place position at left/top of axis rect
    updateArrowAnchor();
}

void NoteItem::updateArrowAnchor()
{
    QPointF diff = _arrow.end->coords() - position->coords();

    if (std::abs(diff.x()) > std::abs(diff.y()))
    {
        if (diff.x() > 0 )
        {
            _arrow.start->setParentAnchor(right);
        }
        else
        {
            _arrow.start->setParentAnchor(left);
        }
    }
    else
    {
        if (diff.y() > 0 )
        {
            _arrow.start->setParentAnchor(top);
        }
        else
        {
            _arrow.start->setParentAnchor(bottom);
        }
    }
}
