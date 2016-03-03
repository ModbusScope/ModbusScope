#include <QMainWindow>
#include <QScrollBar>
#include <QScrollArea>


#include <QDebug>

#include "verticalscrollareacontents.h"

VerticalScrollAreaContents::VerticalScrollAreaContents(QScrollArea *parent) : QWidget(parent)
{

}

void VerticalScrollAreaContents::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event);

    QScrollArea * pParentScrollArea = (QScrollArea *)this->parentWidget();

    pParentScrollArea->parentWidget()->setMinimumWidth(minimumSizeHint().width());

}
