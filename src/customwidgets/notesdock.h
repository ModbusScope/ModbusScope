#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <QObject>
#include <QDockWidget>

// Forward declaration
class NotesDockWidget;
class NoteModel;
class GuiModel;

class NotesDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit NotesDock(NoteModel * pNoteModel, GuiModel * pGuiModel, QWidget *parent = nullptr);
    ~NotesDock();

    void hideEvent(QHideEvent *);
    void showEvent(QShowEvent *);

private:

    void disableNoteDrag();

    NotesDockWidget* _pNotesDockWidget;
    NoteModel * _pNoteModel;

};

#endif // DOCKWIDGET_H
