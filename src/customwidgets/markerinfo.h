#ifndef MARKERINFO_H
#define MARKERINFO_H

#include <QFrame>
#include <QVBoxLayout>

#include "markerinfoitem.h"

class MarkerInfo : public QFrame
{
public:
    MarkerInfo(QWidget *parent = 0);

signals:

public slots:

private slots:

private:

    QVBoxLayout * _pLayout;
    QList<MarkerInfoItem *> _items;

};

#endif // MARKERINFO_H
