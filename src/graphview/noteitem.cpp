#include "noteitem.h"

NoteItem::NoteItem(MyQCustomPlot* pPlot,
                   const QString& rText,
                   const QPointF& rPosition,
                   const QPointF& rArrowPosition)
    : _note(nullptr),
      _arrow(nullptr),
      _plot(pPlot)
{  
    _note = new QCPItemText(_plot);
    _arrow = new QCPItemLine(_plot);

    _note->setPositionAlignment(Qt::AlignBottom|Qt::AlignRight);
    _note->setTextAlignment(Qt::AlignLeft);
    _note->setText(rText);
    _note->position->setType(QCPItemPosition::ptPlotCoords);
    _note->setPen(QPen(Qt::black)); // show black border around text
    _note->position->setCoords(rPosition); // place position at left/top of axis rect

    _arrow->setHead(QCPLineEnding::esSpikeArrow);
    setArrowPosition(rArrowPosition);
}

NoteItem::~NoteItem()
{
    _plot->removeItem(_note);
    _plot->removeItem(_arrow);
}

void NoteItem::setArrowPosition(const QPointF& rPosition)
{
    _arrow->end->setCoords(rPosition);
    updateArrowAnchor();
}

void NoteItem::setNotePosition(const QPointF& rPosition)
{
    _note->position->setCoords(rPosition); // place position at left/top of axis rect
    updateArrowAnchor();
}

void NoteItem::setText(const QString& text)
{
    _note->setText(text);
}

QPoint NoteItem::getNotePosition() const
{
    return _note->topLeft->pixelPosition().toPoint();
}

bool NoteItem::isItem(QCPAbstractItem * pItem) const
{
    return pItem == _note;
}

void NoteItem::updateArrowAnchor()
{
    QPointF diff = _arrow->end->pixelPosition() - _note->topLeft->pixelPosition();

    if (std::abs(diff.x()) > std::abs(diff.y()))
    {
        if (diff.x() > 0)
        {
            _arrow->start->setParentAnchor(_note->right);
        }
        else
        {
            _arrow->start->setParentAnchor(_note->left);
        }
    }
    else
    {
        if (diff.y() > 0)
        {
            _arrow->start->setParentAnchor(_note->bottom);
        }
        else
        {
            _arrow->start->setParentAnchor(_note->top);
        }
    }
}
