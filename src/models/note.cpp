#include "note.h"

Note::Note() :
    _arrowPosition(0.0, 0.0),
    _relativeNotePosition(0.0, 0.0),
    _text(""),
    _bDraggable(false)
{
}

QString Note::text() const
{
    return _text;
}

void Note::setText(const QString &text)
{
    _text = text;
}


const QPointF& Note::arrowPosition() const
{
    return _arrowPosition;
}

void Note::setArrowPosition(const QPointF &position)
{
    _arrowPosition = position;
}

const QPointF& Note::notePosition() const
{
    return _relativeNotePosition;
}

void Note::setNotePosition(const QPointF &position)
{
    _relativeNotePosition = position;
}

void Note::setNotePosition(double key, double value)
{
    setNotePosition(QPointF(key, value));
}

bool Note::draggable() const
{
    return _bDraggable;
}

void Note::setDraggable(bool state)
{
    _bDraggable = state;
}
