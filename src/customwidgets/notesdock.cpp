#include "notesdock.h"

#include "customwidgets/notesdockwidget.h"
#include "models/guimodel.h"
#include "models/notemodel.h"

NotesDock::NotesDock(NoteModel * pNoteModel, GuiModel *pGuiModel, QWidget *parent) :
    QDockWidget(parent)
{

    _pNoteModel = pNoteModel;

    setAllowedAreas(Qt::NoDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetClosable
                | QDockWidget::DockWidgetFloatable
                );

    setWindowTitle("Notes");
    setFloating(true);

    _pNotesDockWidget = new NotesDockWidget(pNoteModel, pGuiModel, this);
    this->setWidget(_pNotesDockWidget);

    this->hide();

}

NotesDock::~NotesDock()
{
    delete _pNotesDockWidget;
}

void NotesDock::hideEvent(QHideEvent *)
{
    disableNoteDrag();
}

void NotesDock::showEvent(QShowEvent *)
{   
    disableNoteDrag();
}

void NotesDock::disableNoteDrag()
{
    for(qint32 idx = 0; idx < _pNoteModel->size(); idx++)
    {
        _pNoteModel->setDraggable(idx, false);
    }
}
