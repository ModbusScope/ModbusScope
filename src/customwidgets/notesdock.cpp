#include "notesdock.h"

NotesDock::NotesDock(NoteModel * pNoteModel, QWidget *parent) :
    QDockWidget(parent)
{

    _pNoteModel = pNoteModel;

    setAllowedAreas(Qt::NoDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetClosable
                | QDockWidget::DockWidgetFloatable
                );

    setWindowTitle("Notes");
    setFloating(true);

    _pNotesDockWidget = new NotesDockWidget(pNoteModel, this);
    this->setWidget(_pNotesDockWidget);

    this->hide();

}

NotesDock::~NotesDock()
{
    delete _pNotesDockWidget;
}

void NotesDock::hideEvent(QHideEvent *)
{
    _pNoteModel->setDraggableMode(false);
}

void NotesDock::showEvent(QShowEvent *)
{
    _pNoteModel->setDraggableMode(true);
}
