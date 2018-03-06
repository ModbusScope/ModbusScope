#include "note.h"

Note::Note()
{
    _valueData = 0;
    _keyData = 0;
    _text = QString("");
    _bDraggable = false;
}

QString Note::text() const
{
    return _text;
}

void Note::setText(const QString &text)
{
    _text = text;
}

double Note::valueData() const
{
    return _valueData;
}

void Note::setValueData(double valueData)
{
    _valueData = valueData;
}

double Note::keyData() const
{
    return _keyData;
}

void Note::setKeyData(double keyData)
{
    _keyData = keyData;
}

bool Note::draggable() const
{
    return _bDraggable;
}

void Note::setDraggable(bool state)
{
    _bDraggable = state;
}
