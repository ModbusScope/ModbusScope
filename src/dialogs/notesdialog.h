#ifndef NOTESDIALOG_H
#define NOTESDIALOG_H

#include <QDialog>
#include "notemodel.h"

namespace Ui {
class notesDialog;
}

class NotesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NotesDialog(NoteModel * pNoteModel, QWidget *parent = 0);
    ~NotesDialog();

private slots:
    void addNoteRow();
    void removeNoteRow();

    void onRegisterInserted(const QModelIndex &parent, int first, int last);

private:
    Ui::notesDialog *_pUi;

    NoteModel * _pNoteModel;
};

#endif // NOTESDIALOG_H
