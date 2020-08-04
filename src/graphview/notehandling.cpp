

#include "graphview.h"
#include "notehandling.h"

const static quint32 NO_DRAGGED_NOTE = 0xFFFFFFFF;

NoteHandling::NoteHandling(NoteModel* pNoteModel, MyQCustomPlot* pPlot, QObject *parent):
    QObject(parent),
    _pNoteModel(pNoteModel),
    _pPlot(pPlot),
    _pGraphview(dynamic_cast<GraphView*>(parent))
{

    // Note model
    connect(_pNoteModel, &NoteModel::notePositionChanged, this, &NoteHandling::handleNotePositionChanged);
    connect(_pNoteModel, &NoteModel::textChanged, this, &NoteHandling::handleNoteTextChanged);
    connect(_pNoteModel, &NoteModel::added, this, &NoteHandling::handleNoteAdded);
    connect(_pNoteModel, &NoteModel::removed, this, &NoteHandling::handleNoteRemoved);

    _pDraggedNoteIdx = NO_DRAGGED_NOTE;
    _pixelOffset.setX(0);
    _pixelOffset.setY(0);
}

NoteHandling::~NoteHandling()
{

}

bool NoteHandling::handleMousePress(QMouseEvent *event)
{
    _pDraggedNoteIdx = NO_DRAGGED_NOTE;
    QCPAbstractItem * pItem = _pPlot->itemAt(event->pos(), false);
    for(int idx = 0; idx < _notesItems.size(); idx++)
    {
        if (_notesItems[idx]->isItem(pItem))
        {
            _pDraggedNoteIdx = idx;
            break;
        }
    }

    if (_pDraggedNoteIdx != NO_DRAGGED_NOTE)
    {
        /* Save cursor offset */
        _pixelOffset = event->pos() - _notesItems[_pDraggedNoteIdx]->getNotePosition();

        return true;
    }

    return false;
}

bool NoteHandling::handleMouseRelease()
{
    _pDraggedNoteIdx = NO_DRAGGED_NOTE;

    return false;
}

bool NoteHandling::handleMouseMove(QMouseEvent *event)
{
    if ((_pDraggedNoteIdx != NO_DRAGGED_NOTE) && _pNoteModel->draggable(_pDraggedNoteIdx))
    {
        _pNoteModel->setNotePostion(_pDraggedNoteIdx, _pGraphview->pixelToPointF(event->pos() - _pixelOffset));

        return true;
    }

    return false;
}

void NoteHandling::handleNotePositionChanged(const quint32 idx)
{
    _notesItems[idx]->setNotePosition(_pNoteModel->notePosition(idx)); // place position at left/top of axis rect
    _pPlot->replot();
}

void NoteHandling::handleNoteTextChanged(const quint32 idx)
{
    _notesItems[idx]->setText(_pNoteModel->textData(idx));
    _pPlot->replot();
}

void NoteHandling::handleNoteAdded(const quint32 idx)
{
    auto newNote = QSharedPointer<NoteItem>( new NoteItem(_pPlot,
                                              _pNoteModel->textData(idx),
                                              _pNoteModel->notePosition(idx)));

    _notesItems.append(newNote);

    _pPlot->replot();
}

void NoteHandling::handleNoteRemoved(const quint32 idx)
{
    _notesItems.removeAt(idx);
    _pPlot->replot();
}
