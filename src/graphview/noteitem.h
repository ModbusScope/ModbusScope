#ifndef NOTEITEM_H
#define NOTEITEM_H

#include <QPoint>

// Forward declaration
class QCPAbstractItem;
class QCPItemText;
class ScopePlot;

class NoteItem
{
public:
    NoteItem(ScopePlot* pPlot,
             const QString& rText,
             const QPointF& rPosition);
    NoteItem(const NoteItem &source);
    virtual ~NoteItem();

    void setNotePosition(const QPointF& rPosition);
    void setText(const QString& text);

    QPoint getNotePosition() const;
    bool isItem(QCPAbstractItem * pItem) const;

private:
    QCPItemText* _note;
    ScopePlot* _plot;
};

#endif // NOTEITEM_H
