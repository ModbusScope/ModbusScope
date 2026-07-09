#include "dialogs/graphmenucontroller.h"

#include "customwidgets/overlaylabel.h"
#include "models/graphdatamodel.h"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QPixmap>

/*!
 * \brief Construct the controller and wire it to the graph data model it reacts to
 * \param pGraphDataModel Graph data model providing the active-graph list
 * \param pGraphMenu Show/hide submenu this controller rebuilds and keeps in sync
 * \param pOverlayLabel Label shown when there are no active graphs
 * \param parent Parent QObject
 *
 * \note The constructor only wires signals; it does not build the initial menu/overlay/action
 *       state. Call handleGraphsCountChanged() once after construction to initialize them.
 */
GraphMenuController::GraphMenuController(GraphDataModel* pGraphDataModel,
                                         QMenu* pGraphMenu,
                                         OverlayLabel* pOverlayLabel,
                                         QObject* parent)
    : QObject(parent), _pGraphDataModel(pGraphDataModel), _pGraphMenu(pGraphMenu), _pOverlayLabel(pOverlayLabel)
{
    connect(_pGraphDataModel, &GraphDataModel::visibilityChanged, this,
            &GraphMenuController::handleGraphVisibilityChange);
    connect(_pGraphDataModel, &GraphDataModel::colorChanged, this, &GraphMenuController::handleGraphColorChange);
    connect(_pGraphDataModel, &GraphDataModel::labelChanged, this, &GraphMenuController::handleGraphLabelChange);
    connect(_pGraphDataModel, &GraphDataModel::activeChanged, this, &GraphMenuController::handleGraphsCountChanged);
    connect(_pGraphDataModel, &GraphDataModel::added, this, &GraphMenuController::handleGraphsCountChanged);
    connect(_pGraphDataModel, &GraphDataModel::removed, this, &GraphMenuController::handleGraphsCountChanged);
    connect(_pGraphDataModel, &GraphDataModel::moved, this, &GraphMenuController::rebuildGraphMenu);
}

/*!
 * \brief Return the show/hide menu action for a graph, or nullptr when it has none
 * \param graphIdx Index of the graph to look up
 * \return The action, or nullptr when the graph is inactive or out of the menu's current bounds
 */
QAction* GraphMenuController::graphAction(GraphIdx graphIdx) const
{
    if (!_pGraphDataModel->isActive(graphIdx))
    {
        return nullptr;
    }

    const ActiveIdx activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);
    if (!activeIdx.isValid() || activeIdx.v >= _pGraphMenu->actions().size())
    {
        return nullptr;
    }

    return _pGraphMenu->actions().at(activeIdx.v);
}

/*!
 * \brief Build a small solid-color icon, as used for both menu rebuild and color updates
 * \param color Color to fill the icon with
 * \return The resulting icon
 */
QIcon GraphMenuController::colorIcon(const QColor& color)
{
    QPixmap pixmap(20, 5);
    pixmap.fill(color);
    return QIcon(pixmap);
}

/*!
 * \brief Slot connected to a show/hide menu action being toggled by the user
 * \param bState New checked state of the action
 */
void GraphMenuController::menuShowHideGraphClicked(bool bState)
{
    QAction* pAction = qobject_cast<QAction*>(QObject::sender());

    const GraphIdx graphIdx = _pGraphDataModel->convertToGraphIndex(pAction->data().value<ActiveIdx>());
    _pGraphDataModel->setVisible(graphIdx, bState);
}

/*!
 * \brief Reflect a model visibility change on the corresponding menu action
 * \param graphIdx Index of the graph whose visibility changed
 */
void GraphMenuController::handleGraphVisibilityChange(GraphIdx graphIdx)
{
    QAction* pAction = graphAction(graphIdx);
    if (pAction == nullptr)
    {
        return;
    }

    pAction->setChecked(_pGraphDataModel->isVisible(graphIdx));
}

/*!
 * \brief Reflect a model color change on the corresponding menu action's icon
 * \param graphIdx Index of the graph whose color changed
 */
void GraphMenuController::handleGraphColorChange(GraphIdx graphIdx)
{
    QAction* pAction = graphAction(graphIdx);
    if (pAction == nullptr)
    {
        return;
    }

    pAction->setIcon(colorIcon(_pGraphDataModel->color(graphIdx)));
}

/*!
 * \brief Reflect a model label change on the corresponding menu action's text
 * \param graphIdx Index of the graph whose label changed
 */
void GraphMenuController::handleGraphLabelChange(GraphIdx graphIdx)
{
    QAction* pAction = graphAction(graphIdx);
    if (pAction == nullptr)
    {
        return;
    }

    pAction->setText(_pGraphDataModel->label(graphIdx));
}

/*!
 * \brief Rebuild the show/hide menu and refresh overlay/action state for the current active-graph count
 */
void GraphMenuController::handleGraphsCountChanged()
{
    rebuildGraphMenu();

    QList<GraphIdx> activeGraphList;
    _pGraphDataModel->activeGraphIndexList(activeGraphList);

    const bool bActiveGraphs = !activeGraphList.isEmpty();
    _pOverlayLabel->setVisible(!bActiveGraphs);

    emit activeGraphsChanged(bActiveGraphs);
}

/*!
 * \brief Regenerate the show/hide submenu from the current active-graph list
 */
void GraphMenuController::rebuildGraphMenu()
{
    _pGraphMenu->clear();

    QList<GraphIdx> activeGraphList;
    _pGraphDataModel->activeGraphIndexList(activeGraphList);

    for (qint32 activeIdx = 0; activeIdx < activeGraphList.size(); activeIdx++)
    {
        const GraphIdx graphIdx = activeGraphList.at(activeIdx);

        QString label = _pGraphDataModel->label(graphIdx);
        QAction* pShowHideAction = _pGraphMenu->addAction(label);

        pShowHideAction->setData(QVariant::fromValue(ActiveIdx(activeIdx)));
        pShowHideAction->setIcon(colorIcon(_pGraphDataModel->color(graphIdx)));
        pShowHideAction->setCheckable(true);
        pShowHideAction->setChecked(_pGraphDataModel->isVisible(graphIdx));

        connect(pShowHideAction, &QAction::toggled, this, &GraphMenuController::menuShowHideGraphClicked);
    }

    _pGraphMenu->setEnabled(!activeGraphList.isEmpty());
}
