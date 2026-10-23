#include "signaleditorpanel.h"
#include "ui_signaleditorpanel.h"

#include "customwidgets/datapointeditorwidget.h"
#include "models/graphdata.h"
#include "models/graphdatamodel.h"

#include <QKeyEvent>
#include <QVBoxLayout>

/*!
 * \brief Constructs the panel with a data point editor for the first configured device.
 * \param pGraphDataModel   Signal list that Add signal appends to.
 * \param pSettingsModel    Pointer to the application settings model.
 * \param pDataPointService Service that sends data point requests to the device's adapter.
 * \param parent            Optional parent widget.
 */
SignalEditorPanel::SignalEditorPanel(GraphDataModel* pGraphDataModel,
                                     SettingsModel* pSettingsModel,
                                     DataPointService* pDataPointService,
                                     QWidget* parent)
    : QWidget(parent), _pUi(new Ui::SignalEditorPanel), _pGraphDataModel(pGraphDataModel)
{
    _pUi->setupUi(this);

    _pEditor = new DataPointEditorWidget(pSettingsModel, pDataPointService, _pUi->addressContainer);
    auto* addressLayout = new QVBoxLayout(_pUi->addressContainer);
    addressLayout->setContentsMargins(0, 0, 0, 0);
    addressLayout->addWidget(_pEditor);

    /* The editor is created after the .ui widgets; move its fields between name and axis */
    setTabOrder(_pUi->lineName, _pEditor);
    setTabOrder(_pEditor, _pUi->radioPrimary);

    _axisGroup.setExclusive(true);
    _axisGroup.addButton(_pUi->radioPrimary);
    _axisGroup.addButton(_pUi->radioSecondary);

    connect(_pUi->btnAdd, &QPushButton::clicked, this, &SignalEditorPanel::handleAdd);
    connect(_pEditor, &DataPointEditorWidget::expressionChanged, this, &SignalEditorPanel::onExpressionChanged);
    connect(_pEditor, &DataPointEditorWidget::errorChanged, this, &SignalEditorPanel::setBuildError);
    connect(_pEditor, &DataPointEditorWidget::deviceChanged, this, &SignalEditorPanel::updateStatus);

    _pUi->btnAdd->setEnabled(_pEditor->hasValidExpression());
    updateStatus();
}

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
    _pUi->lineName->clear();
    _pUi->lineName->setFocus();
}

/*!
 * \brief Enter in any field adds the signal instead of closing the dialog.
 */
void SignalEditorPanel::keyPressEvent(QKeyEvent* event)
{
    if (event->key() != Qt::Key_Return && event->key() != Qt::Key_Enter)
    {
        QWidget::keyPressEvent(event);
        return;
    }

    event->accept();
    handleAdd();
}

/*!
 * \brief Append a signal with the editor's data point reference, then clear the name for the next one.
 */
void SignalEditorPanel::handleAdd()
{
    if (!_pEditor->hasValidExpression())
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
 * \brief Allow Add only while the editor holds a reference built from the current form.
 *
 * Also clears the "Added" message once the form is changed for the next signal.
 */
void SignalEditorPanel::onExpressionChanged()
{
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
 * \brief Show the most relevant message: no device, the build error, or the last added signal.
 */
void SignalEditorPanel::updateStatus()
{
    QString text;
    bool bError = true;
    if (_pEditor->deviceId() == 0)
    {
        text = tr("Add a device in the connection settings first.");
    }
    else if (!_buildError.isEmpty())
    {
        text = _buildError;
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
