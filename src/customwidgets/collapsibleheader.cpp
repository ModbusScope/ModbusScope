#include "collapsibleheader.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QToolButton>

/*!
 * \brief Header of a collapsible section: an arrow with a title, followed by a line across the row.
 *
 * The section starts collapsed. A click anywhere on the row expands or collapses it.
 * \param parent The parent widget.
 */
CollapsibleHeader::CollapsibleHeader(QWidget* parent) : QWidget(parent), _pToggle(new QToolButton(this))
{
    _pToggle->setCheckable(true);
    _pToggle->setAutoRaise(true);
    _pToggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _pToggle->setArrowType(Qt::RightArrow);
    _pToggle->setFocusPolicy(Qt::StrongFocus);

    QFont font = _pToggle->font();
    font.setBold(true);
    _pToggle->setFont(font);

    auto* pLine = new QFrame(this);
    pLine->setFrameShape(QFrame::HLine);
    pLine->setFrameShadow(QFrame::Sunken);

    auto* pLayout = new QHBoxLayout(this);
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->addWidget(_pToggle);
    pLayout->addWidget(pLine, 1);

    setFocusProxy(_pToggle);

    connect(_pToggle, &QToolButton::toggled, this, &CollapsibleHeader::handleToggled);
}

/*!
 * \brief Return the section title.
 */
QString CollapsibleHeader::text() const
{
    return _pToggle->text();
}

/*!
 * \brief Set the section title; an ampersand marks the mnemonic.
 * \param text The title.
 */
void CollapsibleHeader::setText(const QString& text)
{
    if (text == _pToggle->text())
    {
        return;
    }

    _pToggle->setText(text);
    emit textChanged(text);
}

/*!
 * \brief Set the widget that is shown while the section is expanded.
 * \param pContent The content widget.
 */
void CollapsibleHeader::setContent(QWidget* pContent)
{
    if (_pContent != nullptr)
    {
        _pContent->setVisible(false);
    }

    _pContent = pContent;
    if (_pContent != nullptr)
    {
        _pContent->setVisible(isExpanded());
    }
}

/*!
 * \brief Return whether the section is expanded.
 */
bool CollapsibleHeader::isExpanded() const
{
    return _pToggle->isChecked();
}

/*!
 * \brief Expand or collapse the section.
 * \param bExpanded Whether the section is expanded.
 */
void CollapsibleHeader::setExpanded(bool bExpanded)
{
    _pToggle->setChecked(bExpanded);
}

/*!
 * \brief Return the button that expands and collapses the section, for the tab order.
 */
QToolButton* CollapsibleHeader::toggleButton() const
{
    return _pToggle;
}

/*!
 * \brief Toggle the section when the row next to the title is clicked.
 * \param pEvent The mouse event.
 */
void CollapsibleHeader::mousePressEvent(QMouseEvent* pEvent)
{
    if (pEvent->button() != Qt::LeftButton)
    {
        QWidget::mousePressEvent(pEvent);
        return;
    }

    _pToggle->setFocus(Qt::MouseFocusReason);
    _pToggle->toggle();
}

/*!
 * \brief Update the arrow and the content, and report the new state.
 * \param bChecked Whether the section is expanded.
 */
void CollapsibleHeader::handleToggled(bool bChecked)
{
    _pToggle->setArrowType(bChecked ? Qt::DownArrow : Qt::RightArrow);
    if (_pContent != nullptr)
    {
        _pContent->setVisible(bChecked);
    }
    emit expandedChanged(bChecked);
}
