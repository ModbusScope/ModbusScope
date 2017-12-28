#include "note.h"

Note::Note()
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
