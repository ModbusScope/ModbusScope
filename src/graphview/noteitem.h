#ifndef NOTEITEM_H
#define NOTEITEM_H

#include <QPoint>

// Forward declaration
class QCPAbstractItem;
class QCPItemText;
class MyQCustomPlot;

class NoteItem
{
public:
    NoteItem(MyQCustomPlot* pPlot,
             const QString& rText,
             const QPointF& rPosition);
    virtual ~NoteItem();

    void setNotePosition(const QPointF& rPosition);
    void setText(const QString& text);

    QPoint getNotePosition() const;
    bool isItem(QCPAbstractItem * pItem) const;

private:
    QCPItemText* _note;
    MyQCustomPlot* _plot;
};

#endif // NOTEITEM_H
