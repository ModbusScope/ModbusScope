#ifndef NOTESDOCK_H
#define NOTESDOCK_H

#include <QWidget>

// Forward declaration
class NoteModel;
class GuiModel;

namespace Ui {
class NotesDockWidget;
}

class NotesDockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NotesDockWidget(NoteModel *pNoteModel, GuiModel *pGuiModel, QWidget *parent = 0);
    ~NotesDockWidget();

private slots:
    void addNoteRow();
    void removeNoteRow();
    void updateDataFileClicked();
    void notesDataUpdatedChanged();
    void onRegisterInserted(const QModelIndex &parent, int first, int last);

private:
    Ui::NotesDockWidget *_pUi;

    NoteModel * _pNoteModel;
    GuiModel * _pGuiModel;
};

#endif // NOTESDOCK_H
