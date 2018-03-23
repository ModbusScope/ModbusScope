#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <QObject>
#include <QDockWidget>

#include "notemodel.h"
#include "guimodel.h"
#include "notesdockwidget.h"

class NotesDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit NotesDock(NoteModel * pNoteModel, GuiModel * pGuiModel, QWidget *parent = 0);
    ~NotesDock();

    void hideEvent(QHideEvent *);
    void showEvent(QShowEvent *);

private:

    void disableNoteDrag();

    NotesDockWidget* _pNotesDockWidget;
    NoteModel * _pNoteModel;

};

#endif // DOCKWIDGET_H
