#include "notesdialog.h"
#include "ui_notesdialog.h"

#include <QShortcut>

NotesDialog::NotesDialog(NoteModel *pNoteModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::notesDialog)
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    _pNoteModel = pNoteModel;

    // Setup registerView
    _pUi->noteView->setModel(_pNoteModel);
    _pUi->noteView->verticalHeader()->hide();

    /* Don't stretch columns */
    _pUi->noteView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    /* Except following columns */
    _pUi->noteView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    // Select using click, shift and control
    _pUi->noteView->setSelectionBehavior(QAbstractItemView::SelectRows);
    _pUi->noteView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Handle remove button
    connect(_pUi->btnRemoveNote, SIGNAL(clicked(bool)), this, SLOT(removeNoteRow()));

    // Handle delete
    QShortcut* shortcut = new QShortcut(QKeySequence(QKeySequence::Delete), _pUi->noteView);
    connect(shortcut, SIGNAL(activated()), this, SLOT(removeNoteRow()));

}

NotesDialog::~NotesDialog()
{
    delete _pUi;
}

void NotesDialog::addNoteRow()
{
    _pNoteModel->insertRow(_pNoteModel->size());
}

void NotesDialog::onRegisterInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(last);

    /* select the first new row, this will also make the row visible */
    _pUi->noteView->selectRow(first);
}

void NotesDialog::removeNoteRow()
{
    // get list of selected rows
    QItemSelectionModel *selected = _pUi->noteView->selectionModel();
    QModelIndexList rowList = selected->selectedRows();

    foreach(QModelIndex rowIndex, rowList)
    {
        _pNoteModel->removeRow(rowIndex.row());
    }
}
