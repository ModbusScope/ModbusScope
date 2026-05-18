#include <QMainWindow>
#include <QScrollArea>
#include <QScrollBar>

#include "verticalscrollareacontents.h"

VerticalScrollAreaContents::VerticalScrollAreaContents(QScrollArea* parent) : QWidget(parent)
{
}

void VerticalScrollAreaContents::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);

    QScrollArea* pParentScrollArea = qobject_cast<QScrollArea*>(this->parentWidget());

    // Should use verticalbar width, but I get segmentation fault on Linux
    pParentScrollArea->parentWidget()->setMinimumWidth(minimumSizeHint().width() + 20);
}
