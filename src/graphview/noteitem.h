#ifndef NOTEITEM_H
#define NOTEITEM_H

#include "qcustomplot.h"
#include "myqcustomplot.h"

class NoteItem
{
public:
    NoteItem(MyQCustomPlot* pPlot,
             const QString& rText,
             const QPointF& rPosition,
             const QPointF& rArrowPosition);
    virtual ~NoteItem();

    void setArrowPosition(const QPointF& rPosition);
    void setNotePosition(const QPointF& rPosition);
    void setText(const QString& text);

    QPoint getNotePosition() const;
    bool isItem(QCPAbstractItem * pItem) const;

private:
    QCPItemText* _note;
    QCPItemLine* _arrow;
    MyQCustomPlot* _plot;

    void updateArrowAnchor();
};

#endif // NOTEITEM_H
