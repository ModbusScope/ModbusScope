#ifndef FOCUSCHAINHELPERS_H
#define FOCUSCHAINHELPERS_H

#include <QList>
#include <QWidget>

/*! \brief Tab order checks for tests.
 *
 * Tab moves along the window's focus chain, so the order of widgets in that chain is the order the
 * user reaches them with Tab.
 */
namespace FocusChainHelpers {

/*!
 * \brief Return \a widgets in the order of their window's focus chain.
 *
 * Compare the result with \a widgets to check that Tab reaches them in that order.
 */
inline QList<QWidget*> inTabOrder(const QList<QWidget*>& widgets)
{
    QList<QWidget*> ordered;
    if (widgets.isEmpty())
    {
        return ordered;
    }

    QWidget* pStart = widgets.first()->window();
    QWidget* pWidget = pStart;
    do
    {
        if (widgets.contains(pWidget))
        {
            ordered.append(pWidget);
        }
        pWidget = pWidget->nextInFocusChain();
    } while (pWidget != pStart);

    return ordered;
}

} // namespace FocusChainHelpers

#endif // FOCUSCHAINHELPERS_H
