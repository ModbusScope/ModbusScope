#ifndef NOTE_H
#define NOTE_H

#include <QPointF>
#include <QString>

class Note
{
public:
    Note();

    const QPointF& arrowPosition() const;
    const QPointF& notePosition() const;
    QString text() const;
    bool draggable() const;

    void setArrowPosition(const QPointF& position);
    void setNotePosition(const QPointF& position);
    void setNotePosition(double key, double value);
    void setText(const QString &text);
    void setDraggable(bool state);

private:
    QPointF _arrowPosition;
    QPointF _relativeNotePosition;
    QString _text;
    bool _bDraggable;

};

#endif // NOTE_H
