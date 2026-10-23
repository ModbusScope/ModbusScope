#include "signaleditorpanel.h"
#include "ui_signaleditorpanel.h"

#include "customwidgets/datapointeditorwidget.h"
#include "models/datapointdescriptioncache.h"
#include "models/graphdata.h"
#include "models/graphdatamodel.h"
#include "util/util.h"

#include <QJsonObject>
#include <QKeyEvent>
#include <QSignalBlocker>
#include <QVBoxLayout>

using State = DataPointDescription::State;

/*!
 * \brief Constructs the panel in add mode, with a data point editor for the first configured device.
 * \param pGraphDataModel   Signal list that Add signal appends to and edit mode changes.
 * \param pSettingsModel    Pointer to the application settings model.
 * \param pDataPointService Service that sends data point requests to the device's adapter.
 * \param pDescriptionCache Cache with the adapter's description of each data point.
 * \param parent            Optional parent widget.
 */
SignalEditorPanel::SignalEditorPanel(GraphDataModel* pGraphDataModel,
                                     SettingsModel* pSettingsModel,
                                     DataPointService* pDataPointService,
                                     DataPointDescriptionCache* pDescriptionCache,
                                     QWidget* parent)
    : QWidget(parent),
      _pUi(new Ui::SignalEditorPanel),
      _pGraphDataModel(pGraphDataModel),
      _pDescriptionCache(pDescriptionCache)
{
    _pUi->setupUi(this);

    _pEditor = new DataPointEditorWidget(pSettingsModel, pDataPointService, _pUi->addressContainer);
    auto* addressLayout = new QVBoxLayout(_pUi->addressContainer);
    addressLayout->setContentsMargins(0, 0, 0, 0);
    addressLayout->addWidget(_pEditor);

    /* The editor is created after the .ui widgets; move its fields between data point list and axis */
    setTabOrder(_pUi->listDataPoints, _pEditor);
    setTabOrder(_pEditor, _pUi->radioPrimary);

    _axisGroup.setExclusive(true);
    _axisGroup.addButton(_pUi->radioPrimary);
    _axisGroup.addButton(_pUi->radioSecondary);

    connect(_pUi->btnAdd, &QPushButton::clicked, this, &SignalEditorPanel::handleAdd);
    connect(_pUi->btnEditExpression, &QPushButton::clicked, this, &SignalEditorPanel::onEditExpression);
    connect(_pUi->lineName, &QLineEdit::textEdited, this, &SignalEditorPanel::onNameEdited);
    connect(_pUi->lineName, &QLineEdit::editingFinished, this, &SignalEditorPanel::onNameEditingFinished);
    connect(_pUi->radioSecondary, &QRadioButton::toggled, this, &SignalEditorPanel::onAxisToggled);
    connect(_pUi->listDataPoints, &QListWidget::currentRowChanged, this, &SignalEditorPanel::onDataPointSelected);

    connect(_pEditor, &DataPointEditorWidget::expressionChanged, this, &SignalEditorPanel::onExpressionChanged);
    connect(_pEditor, &DataPointEditorWidget::errorChanged, this, &SignalEditorPanel::setBuildError);
    connect(_pEditor, &DataPointEditorWidget::deviceChanged, this, &SignalEditorPanel::updateStatus);

    connect(_pGraphDataModel, &GraphDataModel::dataChanged, this, &SignalEditorPanel::onModelDataChanged);
    connect(_pGraphDataModel, &GraphDataModel::rowsRemoved, this, &SignalEditorPanel::onRowsRemoved);
    connect(_pGraphDataModel, &GraphDataModel::modelReset, this, &SignalEditorPanel::onRowsRemoved);
    connect(_pDescriptionCache, &DataPointDescriptionCache::descriptionChanged, this,
            &SignalEditorPanel::onDescriptionChanged);

    setEditWidgetsVisible(false);
    _pUi->btnAdd->setEnabled(_pEditor->hasValidExpression());
    updateStatus();
}

//! Destroys the panel and its generated UI.
SignalEditorPanel::~SignalEditorPanel()
{
    delete _pUi;
}

/*!
 * \brief Prepare the panel for a new signal: clear the name and give it the focus.
 *
 * Device, address fields and axis are kept, so a series of signals only needs the changed fields.
 */
void SignalEditorPanel::startAdd()
{
    showAddMode();

    _pUi->lineName->clear();
    _pUi->lineName->setFocus();
}

/*!
 * \brief Leave edit mode without moving the focus.
 *
 * The editor keeps the device and fields of the edited data point, as it does after an add.
 */
void SignalEditorPanel::showAddMode()
{
    if (!_bEditMode)
    {
        return;
    }

    _bEditMode = false;
    _editedRow = QPersistentModelIndex();
    _references.clear();
    _knownExpression.clear();
    _bAwaitingDescription = false;

    _pUi->lineName->clear();
    _pEditor->setEnabled(true);
    setEditWidgetsVisible(false);

    _pUi->btnAdd->setEnabled(_pEditor->hasValidExpression());
    updateStatus();
}

/*!
 * \brief Show a signal in the panel; name, axis and data points then edit that row of the model.
 * \param graphIdx The row to edit.
 */
void SignalEditorPanel::editSignal(GraphIdx graphIdx)
{
    const QModelIndex index = _pGraphDataModel->index(graphIdx.v, 0);
    if (!index.isValid())
    {
        showAddMode();
        return;
    }

    if (!_bEditMode || _editedRow != index)
    {
        _referenceIdx = 0;
        _references.clear();
    }

    _bEditMode = true;
    _editedRow = QPersistentModelIndex(index);
    _addedMessage.clear();

    setEditWidgetsVisible(true);
    refresh(true);
}

/*!
 * \brief Enter in any field adds the signal instead of closing the dialog; in edit mode it does nothing.
 */
void SignalEditorPanel::keyPressEvent(QKeyEvent* event)
{
    if (event->key() != Qt::Key_Return && event->key() != Qt::Key_Enter)
    {
        QWidget::keyPressEvent(event);
        return;
    }

    event->accept();
    if (!_bEditMode)
    {
        handleAdd();
    }
}

/*!
 * \brief Append a signal with the editor's data point reference, then clear the name for the next one.
 */
void SignalEditorPanel::handleAdd()
{
    if (_bEditMode || !_pEditor->hasValidExpression())
    {
        return;
    }

    GraphData graphData;
    graphData.setLabel(_pUi->lineName->text());
    graphData.setValueAxis(_pUi->radioSecondary->isChecked() ? GraphData::VALUE_AXIS_SECONDARY
                                                             : GraphData::VALUE_AXIS_PRIMARY);
    graphData.setExpression(_pEditor->expression());
    _pGraphDataModel->add(graphData);

    const QString label = _pGraphDataModel->label(GraphIdx(_pGraphDataModel->size() - 1));
    _addedMessage = tr("Added \"%1\"").arg(label);
    updateStatus();

    startAdd();
}

/*!
 * \brief Follow a new reference from the editor.
 *
 * In add mode, Add is only allowed while the editor holds a reference built from the current form;
 * a form change also clears the "Added" message. In edit mode, the reference replaces the edited
 * occurrence in the row's expression; the rest of the expression is kept.
 */
void SignalEditorPanel::onExpressionChanged()
{
    if (_bEditMode)
    {
        if (!hasEditedRow() || !_pEditor->hasValidExpression() || _references.isEmpty())
        {
            return;
        }

        const ExpressionReferences::Reference& reference = _references.at(_referenceIdx);
        const QString newText = _pEditor->expression();
        if (newText == reference.text)
        {
            return;
        }

        const QString expression = _pGraphDataModel->expression(editedGraph());
        const QString newExpression = ExpressionReferences::replace(expression, reference, newText);
        if (newExpression == expression)
        {
            return;
        }

        _knownExpression = newExpression;
        _pGraphDataModel->setExpression(editedGraph(), newExpression);
        return;
    }

    _pUi->btnAdd->setEnabled(_pEditor->hasValidExpression());

    /* A field or device change started the next signal; the form no longer shows the added one */
    if (!_pEditor->hasValidExpression() && !_addedMessage.isEmpty())
    {
        _addedMessage.clear();
        updateStatus();
    }
}

/*!
 * \brief Show why the data point reference could not be built, or clear the message.
 * \param error The reason the request failed; empty when there is no error.
 */
void SignalEditorPanel::setBuildError(const QString& error)
{
    _buildError = error;
    if (!error.isEmpty())
    {
        _addedMessage.clear();
    }
    updateStatus();
}

/*!
 * \brief Show the most relevant message.
 *
 * Add mode: no device, the build error, or the last added signal. Edit mode: the syntax error,
 * an expression without data points, no device, the build error, or why the edited data point has
 * no description.
 */
void SignalEditorPanel::updateStatus()
{
    QString text;
    bool bError = true;
    if (hasEditedRow() && _pGraphDataModel->expressionState(editedGraph()) == GraphData::ExpressionState::SYNTAX_ERROR)
    {
        text = _pGraphDataModel->expressionError(editedGraph());
    }
    else if (_bEditMode && _references.isEmpty())
    {
        text = tr("This signal reads no data point.");
        bError = false;
    }
    else if (_pEditor->deviceId() == 0)
    {
        text = tr("Add a device in the connection settings first.");
    }
    else if (!_buildError.isEmpty())
    {
        text = _buildError;
    }
    else if (_bEditMode)
    {
        text = referenceProblem();
    }
    else
    {
        text = _addedMessage;
        bError = false;
    }

    _pUi->lblStatus->setStyleSheet(bError ? QStringLiteral("QLabel { color: #c00000; }") : QString());
    _pUi->lblStatus->setText(text);
    _pUi->lblStatus->setVisible(!text.isEmpty());
}

/*!
 * \brief Rename the edited signal while typing; an empty name is not applied.
 * \param name The text in the name field.
 */
void SignalEditorPanel::onNameEdited(const QString& name)
{
    if (!hasEditedRow() || name.isEmpty())
    {
        return;
    }

    _pGraphDataModel->setLabel(editedGraph(), name);
}

/*!
 * \brief Show the signal's name again when the name field is left empty.
 */
void SignalEditorPanel::onNameEditingFinished()
{
    if (!hasEditedRow() || !_pUi->lineName->text().isEmpty())
    {
        return;
    }

    _pUi->lineName->setText(_pGraphDataModel->label(editedGraph()));
}

/*!
 * \brief Move the edited signal to the selected axis.
 */
void SignalEditorPanel::onAxisToggled()
{
    if (!hasEditedRow())
    {
        return;
    }

    _pGraphDataModel->setValueAxis(editedGraph(), _pUi->radioSecondary->isChecked() ? GraphData::VALUE_AXIS_SECONDARY
                                                                                    : GraphData::VALUE_AXIS_PRIMARY);
}

/*!
 * \brief Load the data point selected in the list into the editor.
 * \param row The occurrence in the expression.
 */
void SignalEditorPanel::onDataPointSelected(int row)
{
    if (row < 0 || row >= _references.size() || row == _referenceIdx)
    {
        return;
    }

    _referenceIdx = row;
    updateExpressionLabel();
    loadReference();
    updateStatus();
}

/*!
 * \brief Ask for Compose expression on the edited signal.
 */
void SignalEditorPanel::onEditExpression()
{
    if (!hasEditedRow())
    {
        return;
    }

    emit editExpressionRequested(editedGraph());
}

/*!
 * \brief Update the panel when the edited row changes.
 *
 * The panel's own expression change keeps the editor as it is, so the field being edited keeps its
 * value and focus. Any other expression change loads the data point again.
 */
void SignalEditorPanel::onModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    if (!hasEditedRow())
    {
        return;
    }

    const int row = _editedRow.row();
    if (row < topLeft.row() || row > bottomRight.row())
    {
        return;
    }

    refresh(_pGraphDataModel->expression(editedGraph()) != _knownExpression);
}

/*!
 * \brief Return to add mode when the edited row was removed.
 */
void SignalEditorPanel::onRowsRemoved()
{
    if (_bEditMode && !_editedRow.isValid())
    {
        showAddMode();
    }
}

/*!
 * \brief Show a description that arrived, and load it into the editor when it still shows that reference.
 * \param referenceText The reference that was described.
 */
void SignalEditorPanel::onDescriptionChanged(const QString& referenceText)
{
    if (!_bEditMode || _references.isEmpty())
    {
        return;
    }

    bool bUsed = false;
    for (const ExpressionReferences::Reference& reference : std::as_const(_references))
    {
        if (reference.text == referenceText)
        {
            bUsed = true;
            break;
        }
    }
    if (!bUsed)
    {
        return;
    }

    updateDataPointList();

    const QString currentText = _references.at(_referenceIdx).text;
    if (_bAwaitingDescription && currentText == referenceText)
    {
        loadReference();
    }

    updateStatus();
}

/*!
 * \brief Returns whether the panel edits a row that still exists.
 */
bool SignalEditorPanel::hasEditedRow() const
{
    return _bEditMode && _editedRow.isValid();
}

/*!
 * \brief Returns the edited row; only meaningful while hasEditedRow() is true.
 */
GraphIdx SignalEditorPanel::editedGraph() const
{
    return GraphIdx(_editedRow.row());
}

/*!
 * \brief Switch the widgets between add mode and edit mode.
 * \param bVisible True for edit mode.
 */
void SignalEditorPanel::setEditWidgetsVisible(bool bVisible)
{
    _pUi->lblTitle->setText(bVisible ? tr("Edit signal") : tr("New signal"));
    _pUi->btnAdd->setVisible(!bVisible);
    _pUi->btnEditExpression->setVisible(bVisible);

    _pUi->addressContainer->setVisible(true);
    _pUi->lblDataPoints->setVisible(false);
    _pUi->listDataPoints->setVisible(false);
    _pUi->lblExpression->setVisible(false);
}

/*!
 * \brief Show the edited row: name, axis and its data points.
 * \param bReloadEditor Load the edited data point into the editor; false keeps what the editor shows.
 */
void SignalEditorPanel::refresh(bool bReloadEditor)
{
    const GraphIdx graphIdx = editedGraph();

    const QString label = _pGraphDataModel->label(graphIdx);
    /* A name field being cleared to type a new name is left alone */
    if (_pUi->lineName->text() != label && !_pUi->lineName->hasFocus())
    {
        const QSignalBlocker blocker(_pUi->lineName);
        _pUi->lineName->setText(label);
    }

    {
        const QSignalBlocker blocker(_pUi->radioSecondary);
        const bool bSecondary = _pGraphDataModel->valueAxis(graphIdx) == GraphData::VALUE_AXIS_SECONDARY;
        _pUi->radioSecondary->setChecked(bSecondary);
        _pUi->radioPrimary->setChecked(!bSecondary);
    }

    const QString expression = _pGraphDataModel->expression(graphIdx);
    _knownExpression = expression;

    /* The editor was hidden, so it does not show a data point of this expression yet */
    if (_references.isEmpty())
    {
        bReloadEditor = true;
    }

    if (_pGraphDataModel->expressionState(graphIdx) == GraphData::ExpressionState::SYNTAX_ERROR)
    {
        _references.clear();
    }
    else
    {
        _references = ExpressionReferences::find(expression);
    }

    const bool bHasReferences = !_references.isEmpty();
    _pUi->addressContainer->setVisible(bHasReferences);
    if (bHasReferences)
    {
        _referenceIdx = qBound(qsizetype(0), _referenceIdx, _references.size() - 1);
    }

    updateDataPointList();
    updateExpressionLabel();

    if (bHasReferences && bReloadEditor)
    {
        loadReference();
    }

    updateStatus();
}

/*!
 * \brief Show the edited data point in the editor, with its fields when the adapter described it.
 *
 * The editor is disabled until the description arrives.
 */
void SignalEditorPanel::loadReference()
{
    /* A copy: the editor's signals reach slots that may find the references again */
    const ExpressionReferences::Reference reference = _references.at(_referenceIdx);
    const DataPointDescription description = _pDescriptionCache->lookup(reference);

    QJsonObject fields;
    if (description.state == State::Valid)
    {
        fields = description.fields;
    }
    if (!fields.contains(QStringLiteral("deviceId")))
    {
        fields.insert(QStringLiteral("deviceId"), static_cast<int>(reference.deviceId));
    }

    _bAwaitingDescription = description.state == State::Pending;
    _pEditor->setReference(reference.text, fields);
    _pEditor->setEnabled(!_bAwaitingDescription);
}

/*!
 * \brief Fill the list of data points; it is only shown when the expression has more than one.
 */
void SignalEditorPanel::updateDataPointList()
{
    const bool bShowList = _references.size() > 1;
    _pUi->lblDataPoints->setVisible(bShowList);
    _pUi->listDataPoints->setVisible(bShowList);
    if (!bShowList)
    {
        return;
    }

    _pUi->lblDataPoints->setText(tr("This signal combines %1 data points.").arg(_references.size()));

    const QSignalBlocker blocker(_pUi->listDataPoints);
    if (_pUi->listDataPoints->count() != _references.size())
    {
        _pUi->listDataPoints->clear();
        for (qsizetype idx = 0; idx < _references.size(); idx++)
        {
            _pUi->listDataPoints->addItem(QString());
        }
    }

    for (qsizetype idx = 0; idx < _references.size(); idx++)
    {
        const ExpressionReferences::Reference& reference = _references.at(idx);
        const DataPointDescription description = _pDescriptionCache->lookup(reference);

        QString text = reference.text;
        bool bError = false;
        if (description.state == State::Valid && !description.description.isEmpty())
        {
            text = QStringLiteral("%1  %2").arg(reference.text, description.description);
        }
        else if (description.state == State::Invalid)
        {
            const QString error = description.error.isEmpty() ? tr("Not a valid data point") : description.error;
            text = QStringLiteral("%1  %2").arg(reference.text, error);
            bError = true;
        }

        QListWidgetItem* pItem = _pUi->listDataPoints->item(static_cast<int>(idx));
        pItem->setText(text);
        pItem->setToolTip(Util::plainTextToolTip(text));
        pItem->setForeground(bError ? QBrush(Qt::red) : QBrush());
    }

    _pUi->listDataPoints->setCurrentRow(static_cast<int>(_referenceIdx));
}

/*!
 * \brief Show the expression with the edited data point in bold, when the expression is more than that data point.
 */
void SignalEditorPanel::updateExpressionLabel()
{
    const QString expression = _pGraphDataModel->expression(editedGraph());
    if (_references.isEmpty() || (_references.size() == 1 && expression.trimmed() == _references.first().text))
    {
        _pUi->lblExpression->setVisible(false);
        return;
    }

    const ExpressionReferences::Reference& reference = _references.at(_referenceIdx);
    const QString before = expression.left(reference.start);
    const QString after = expression.mid(reference.start + reference.length);

    /* The expression can come from a project file: escape it, only the mark is markup */
    _pUi->lblExpression->setText(QStringLiteral("<span style=\"white-space: pre-wrap\">%1<b>%2</b>%3</span>")
                                   .arg(before.toHtmlEscaped(), reference.text.toHtmlEscaped(), after.toHtmlEscaped()));
    _pUi->lblExpression->setVisible(true);
}

/*!
 * \brief Returns why the edited data point has no description, or an empty string.
 *
 * Only while the editor still shows that data point: a changed field replaces it anyway.
 */
QString SignalEditorPanel::referenceProblem() const
{
    if (_references.isEmpty())
    {
        return QString();
    }

    const ExpressionReferences::Reference& reference = _references.at(_referenceIdx);
    if (_pEditor->expression() != reference.text)
    {
        return QString();
    }

    const DataPointDescription description = _pDescriptionCache->lookup(reference);
    switch (description.state)
    {
    case State::Invalid:
        return tr("The adapter rejects this data point: %1. Change a field to replace it.")
          .arg(description.error.isEmpty() ? tr("Not a valid data point") : description.error);
    case State::Failed:
        return tr("The data point could not be described: %1").arg(description.error);
    case State::Unavailable:
        return tr("The adapter of this device is not running.");
    default:
        return QString();
    }
}
