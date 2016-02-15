#ifndef MARKERINFOITEM_H
#define MARKERINFOITEM_H

#include <QLabel>
#include <QHBoxLayout>

class MarkerInfoItem : public QFrame
{
public:
    MarkerInfoItem(QWidget *parent = 0);

signals:

public slots:

private:

    QHBoxLayout * _pLayout;
    QLabel * _pLabel;

};

#endif // MARKERINFOITEM_H
