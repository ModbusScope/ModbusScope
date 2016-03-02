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

    //qDebug() << "Minimum size hint (scroll contents): " << minimumSizeHint();
    pParentScrollArea->setMinimumWidth(minimumSizeHint().width() /* + pParentScrollArea->verticalScrollBar()->width()*/);

    pParentScrollArea->updateGeometry();

    // dirty dirty test
    //pParentScrollArea->parentWidget()->parentWidget()->updateGeometry();

    qDebug() << "Minimum size (scrollarea): " << pParentScrollArea->minimumSize();

}
