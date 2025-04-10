#ifndef VERTICALSCROLLAREA_H
#define VERTICALSCROLLAREA_H

#include <QObject>
#include <QWidget>
#include <QScrollArea>

class VerticalScrollAreaContents : public QWidget
{

    Q_OBJECT

public:
    explicit VerticalScrollAreaContents(QScrollArea *parent = nullptr);

    void resizeEvent(QResizeEvent * event);

};

#endif // VERTICALSCROLLAREA_H
