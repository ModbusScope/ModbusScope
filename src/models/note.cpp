#include "note.h"

Note::Note() :
    _relativeNotePosition(0.0, 0.0),
    _text(""),
    _bDraggable(false)
{
}

Note::Note(const QString& text, const QPointF& position)
    : _relativeNotePosition(position),
      _text(text),
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
