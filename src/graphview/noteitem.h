#ifndef NOTEITEM_H
#define NOTEITEM_H

#include "qcustomplot.h"
#include "myqcustomplot.h"

class NoteItem : public QCPItemText
{
public:
    NoteItem(MyQCustomPlot* pPlot,
             const QString& rText,
             const QPointF& rPosition,
             const QPointF& rArrowPosition);
    virtual ~NoteItem();

    void setArrowPosition(const QPointF& rPosition);
    void setNotePosition(const QPointF& rPosition);

private:
    QCPItemLine _arrow;

    void updateArrowAnchor();
};

#endif // NOTEITEM_H
