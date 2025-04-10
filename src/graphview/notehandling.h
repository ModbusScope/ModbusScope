#ifndef NOTEHANDLING_H
#define NOTEHANDLING_H

#include <QObject>
#include <QMouseEvent>

#include "graphview/noteitem.h"

// Forward declaration
class GraphView;
class NoteModel;
class ScopePlot;

class NoteHandling : public QObject
{
    Q_OBJECT
public:

    explicit NoteHandling(NoteModel* pNoteModel, ScopePlot* pPlot, QObject *parent = nullptr);
    virtual ~NoteHandling();

    bool handleMousePress(QMouseEvent *event);
    bool handleMouseRelease();
    bool handleMouseWheel();
    bool handleMouseMove(QMouseEvent *event);

public slots:
    void handleNotePositionChanged(const quint32 idx);
    void handleNoteTextChanged(const quint32 idx);
    void handleNoteAdded(const quint32 idx);
    void handleNoteRemoved(const quint32 idx);

private:

    NoteModel* _pNoteModel;
    ScopePlot* _pPlot;
    GraphView* _pGraphview;

    QList< QSharedPointer<NoteItem> > _notesItems;
    quint32 _pDraggedNoteIdx;
    QPoint _pixelOffset;

};

#endif // NOTEHANDLING_H
