#ifndef NOTE_H
#define NOTE_H

#include <QPointF>
#include <QString>

class Note
{
public:
    Note();
    Note(const QString& text, const QPointF& position);

    const QPointF& position() const;
    QString text() const;
    bool draggable() const;

    void setPosition(const QPointF& position);
    void setPosition(double key, double value);
    void setText(const QString &text);
    void setDraggable(bool state);

private:
    QPointF _relativeNotePosition;
    QString _text;
    bool _bDraggable;

};

#endif // NOTE_H
