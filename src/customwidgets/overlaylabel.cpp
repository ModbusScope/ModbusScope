#include "overlaylabel.h"

#include <QEvent>
#include <QLabel>
#include <QWidget>

/*!
 * \brief Constructs an OverlayLabel centered over \a pParent.
 *
 * \param text     The message to display.
 * \param pParent  The widget to overlay; owns the label as a child.
 * \param parent   QObject parent for memory management.
 */
OverlayLabel::OverlayLabel(const QString& text, QWidget* pParent, QObject* parent)
    : QObject(parent), _pTargetWidget(pParent), _pLabel(new QLabel(text, pParent))
{
    Q_ASSERT(pParent != nullptr);

    _pLabel->setAlignment(Qt::AlignCenter);
    _pLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    _pLabel->setStyleSheet("color: #888888; font-size: 13px;");
    _pLabel->resize(pParent->size());
    _pLabel->setVisible(false);

    pParent->installEventFilter(this);
}

/*!
 * \brief Shows or hides the overlay label.
 * \param visible Pass \c true to show, \c false to hide.
 */
void OverlayLabel::setVisible(bool visible)
{
    _pLabel->setVisible(visible);
}

/*!
 * \brief Resizes the overlay label to match the target widget on resize events.
 */
bool OverlayLabel::eventFilter(QObject* watched, QEvent* event)
{
    if (!_pTargetWidget.isNull() && watched == _pTargetWidget && event->type() == QEvent::Resize)
    {
        _pLabel->resize(_pTargetWidget->size());
    }
    return QObject::eventFilter(watched, event);
}
