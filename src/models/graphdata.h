#ifndef GRAPHDATA_H
#define GRAPHDATA_H

#include <QObject>
#include <QColor>

class GraphData : public QObject
{
    Q_OBJECT

public:

    bool bVisibility;
    QString label;
    QColor color;

private:

};

#endif // GRAPHDATA_H
