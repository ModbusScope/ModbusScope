#include "notesdockwidget.h"
#include "ui_notesdockwidget.h"

#include <QShortcut>

NotesDockWidget::NotesDockWidget(NoteModel *pNoteModel, GuiModel *pGuiModel, QWidget *parent) :
    QWidget(parent),
    _pUi(new Ui::NotesDockWidget),
    _pNoteModel(pNoteModel),
    _pGuiModel(pGuiModel)
{
    _pUi->setupUi(this);

    // Setup registerView
    _pUi->noteView->setModel(_pNoteModel);
    _pUi->noteView->verticalHeader()->hide();

    /* Don't stretch columns */
    _pUi->noteView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    /* Except following columns */
    _pUi->noteView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    // Select using click, shift and control
    _pUi->noteView->setSelectionBehavior(QAbstractItemView::SelectRows);
    _pUi->noteView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    _pUi->btnUpdateDataFile->setVisible(false);

    connect(_pGuiModel, &GuiModel::guiStateChanged, this, &NotesDockWidget::notesDataUpdatedChanged);
    connect(_pNoteModel, &NoteModel::notesDataUpdatedChanged, this, &NotesDockWidget::notesDataUpdatedChanged);

    connect(_pUi->btnUpdateDataFile, &QPushButton::clicked, this, &NotesDockWidget::updateDataFileClicked);

    // Handle remove button
    connect(_pUi->btnRemoveNote, &QPushButton::clicked, this, &NotesDockWidget::removeNoteRow);

    // Handle delete
    QShortcut* shortcut = new QShortcut(QKeySequence(QKeySequence::Delete), _pUi->noteView);
    connect(shortcut, &QShortcut::activated, this, &NotesDockWidget::removeNoteRow);
}

NotesDockWidget::~NotesDockWidget()
{
    delete _pUi;
}

void NotesDockWidget::addNoteRow()
{
    _pNoteModel->insertRow(_pNoteModel->size());
}

void NotesDockWidget::onRegisterInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(last);

    /* select the first new row, this will also make the row visible */
    _pUi->noteView->selectRow(first);
}

void NotesDockWidget::removeNoteRow()
{
    QList <int> rows;
    QModelIndexList list = _pUi->noteView->selectionModel()->selectedRows();
    for (const QModelIndex &idx: std::as_const(list))
    {
        rows << idx.row();
    }

    std::sort(rows.begin(), rows.end(), std::greater<int>());

    for (int i: std::as_const(rows))
    {
        _pNoteModel->removeRow(i);
    }
}

void NotesDockWidget::updateDataFileClicked()
{
    emit _pNoteModel->dataFileUpdateRequested();
}

void NotesDockWidget::notesDataUpdatedChanged()
{
    _pUi->btnUpdateDataFile->setVisible(_pGuiModel->guiState() == GuiModel::GuiState::DATA_LOADED);
    _pUi->btnUpdateDataFile->setEnabled(_pNoteModel->isNotesDataUpdated());
}


