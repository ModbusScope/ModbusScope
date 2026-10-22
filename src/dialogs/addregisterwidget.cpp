#include "addregisterwidget.h"
#include "ui_addregisterwidget.h"

#include "customwidgets/datapointeditorwidget.h"
#include "models/graphdata.h"

#include <QCoreApplication>
#include <QMenu>
#include <QResizeEvent>
#include <QVBoxLayout>

/*!
 * \brief Constructs the widget with a data point editor for the first configured device.
 * \param pSettingsModel    Pointer to the application settings model.
 * \param pDataPointService Service that sends data point requests to the device's adapter.
 * \param parent            Optional parent widget.
 */
AddRegisterWidget::AddRegisterWidget(SettingsModel* pSettingsModel,
                                     DataPointService* pDataPointService,
                                     QWidget* parent)
    : QWidget(parent),
      _pUi(new Ui::AddRegisterWidget),
      _pEditor(new DataPointEditorWidget(pSettingsModel, pDataPointService, this))
{
    _pUi->setupUi(this);

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto* addressLayout = new QVBoxLayout(_pUi->addressContainer);
    addressLayout->setContentsMargins(0, 0, 0, 0);
    addressLayout->addWidget(_pEditor);

    _pUi->lblError->setStyleSheet("QLabel { color: #c00000; }");
    _pUi->lblError->setVisible(false);

    _axisGroup.setExclusive(true);
    _axisGroup.addButton(_pUi->radioPrimary);
    _axisGroup.addButton(_pUi->radioSecondary);

    connect(_pUi->btnAdd, &QPushButton::clicked, this, &AddRegisterWidget::handleAdd);
    connect(_pEditor, &DataPointEditorWidget::expressionChanged, this, &AddRegisterWidget::onExpressionChanged);
    connect(_pEditor, &DataPointEditorWidget::errorChanged, this, &AddRegisterWidget::setBuildError);
    connect(_pEditor, &DataPointEditorWidget::deviceChanged, this, &AddRegisterWidget::resizeContainingMenu);

    _pUi->btnAdd->setEnabled(_pEditor->hasValidExpression());
    resetFields();
}

AddRegisterWidget::~AddRegisterWidget()
{
    delete _pUi;
}

/*!
 * \brief Emit the configured signal with the editor's data point reference, then reset the form.
 */
void AddRegisterWidget::handleAdd()
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
    emit graphDataConfigured(graphData);

    resetFields();
    _pEditor->setDevice(_pEditor->deviceId());

    QMetaObject::invokeMethod(this, &AddRegisterWidget::closeContainingMenu, Qt::QueuedConnection);
}

/*!
 * \brief Allow Add only while the editor holds a reference built from the current form.
 */
void AddRegisterWidget::onExpressionChanged()
{
    _pUi->btnAdd->setEnabled(_pEditor->hasValidExpression());
    resizeContainingMenu();
}

/*!
 * \brief Resizes the enclosing popup menu to fit this widget's current content.
 *
 * When hosted as a QWidgetAction's default widget inside a QToolButton's popup menu (as
 * SignalsDialog does for btnAdd), QMenu caches its action layout and only recomputes it for
 * specific events (its action list changing, being shown, or being resized) - it has no way to
 * know an already-embedded widget's own content changed. Without this, switching devices while
 * the popup is open leaves it clipped to whichever device's form was shown first. A synthetic resize
 * event forces QMenu to mark its cached layout dirty and recompute it, so the sizeHint() below
 * reflects the rebuilt form instead of stale, cached action rects.
 */
void AddRegisterWidget::resizeContainingMenu()
{
    auto* menu = qobject_cast<QMenu*>(window());
    if (menu == nullptr)
    {
        return;
    }

    QResizeEvent resizeEvent(menu->size(), menu->size());
    QCoreApplication::sendEvent(menu, &resizeEvent);

    menu->resize(menu->sizeHint());
}

/*!
 * \brief Closes the enclosing popup menu, if any, after a signal was added.
 *
 * When hosted as a QWidgetAction's default widget inside a QToolButton's popup menu (as
 * SignalsDialog does for btnAdd), clicking this widget's own "Add" button doesn't trigger
 * the QWidgetAction itself, so QMenu never closes on its own. Close it explicitly so adding
 * a signal behaves like any other menu action.
 *
 * Called through a queued invocation from handleAdd(), so closing the menu never nests inside
 * btnAdd's own click handling.
 */
void AddRegisterWidget::closeContainingMenu()
{
    auto* menu = qobject_cast<QMenu*>(window());
    if (menu == nullptr)
    {
        return;
    }

    menu->close();
}

/*!
 * \brief Show why the data point reference could not be built, or hide the message.
 *
 * The message is shown as plain text. Also resizes the enclosing popup menu, if any, to fit it.
 * \param error The reason the request failed; empty to hide the message.
 */
void AddRegisterWidget::setBuildError(const QString& error)
{
    if (_pUi->lblError->text() == error)
    {
        return;
    }

    _pUi->lblError->setText(error);
    _pUi->lblError->setVisible(!error.isEmpty());

    /* The label ignores its own width, so a long message wraps within the form instead of
     * widening the popup. QMenu only uses size hints, so reserve the wrapped height explicitly. */
    const int formWidth = _pUi->groupBox->sizeHint().width();
    _pUi->lblError->setMinimumHeight(error.isEmpty() ? 0 : _pUi->lblError->heightForWidth(formWidth));
    resizeContainingMenu();
}

void AddRegisterWidget::resetFields()
{
    _pUi->lineName->clear();
    _pUi->lineName->setFocus();
    _pUi->radioPrimary->setChecked(true);
}
