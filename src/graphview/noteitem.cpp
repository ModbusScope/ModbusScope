
#include "qcustomplot.h"
#include "scopeplot.h"

#include "noteitem.h"

NoteItem::NoteItem(ScopePlot* pPlot,
                   const QString& rText,
                   const QPointF& rPosition)
    : _note(nullptr),
      _plot(pPlot)
{  
    _note = new QCPItemText(_plot);

    _note->setPositionAlignment(Qt::AlignTop|Qt::AlignLeft);
    _note->setTextAlignment(Qt::AlignLeft);
    _note->setText(rText);
    _note->position->setType(QCPItemPosition::ptPlotCoords);
    _note->setPen(QPen(Qt::black)); // show black border around text
    _note->position->setCoords(rPosition); // place position at left/top of axis rect
}

NoteItem::NoteItem(const NoteItem &source)
    : _note(source._note),
      _plot(source._plot)
{
    /* intended shallow copy: plot takes ownership of QCPItemText (_note) */
}

NoteItem::~NoteItem()
{
    _plot->removeItem(_note);
}

void NoteItem::setNotePosition(const QPointF& rPosition)
{
    _note->position->setCoords(rPosition); // place position at left/top of axis rect
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
