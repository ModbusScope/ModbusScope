#ifndef SCOPEGUI_H
#define SCOPEGUI_H

#include <QObject>

// Foward declaration
class QCustomPlot;

class ScopeGui : public QObject
{
    Q_OBJECT
public:
    explicit ScopeGui(QCustomPlot * pGraph, QObject *parent);

    void ResetGraph(quint32 variableCount);

signals:

public slots:
    void PlotResults(bool bSuccess, QList<quint16> values);

private slots:
    void SelectionChanged();
    void MousePress();
    void MouseWheel();

private:


    QCustomPlot * _pGraph;
};

#endif // SCOPEGUI_H
