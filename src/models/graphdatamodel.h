#ifndef GRAPHDATAMODEL_H
#define GRAPHDATAMODEL_H

#include <QObject>
#include <QList>

#include "graphdata.h"


class GraphDataModel : public QObject
{
    Q_OBJECT
public:
    explicit GraphDataModel(QObject *parent = 0);

    qint32 size();
    qint32 activeCount();

    bool isVisible(quint32 index) const;
    QString label(quint32 index) const;
    QColor color(quint32 index) const;
    bool isActive(quint32 index) const;
    bool isUnsigned(quint32 index) const;
    double multiplyFactor(quint32 index) const;
    double divideFactor(quint32 index) const;
    quint16 registerAddress(quint32 index) const;
    quint16 bitmask(quint32 index) const;
    qint32 shift(quint32 index) const;

    void setVisible(quint32 index, bool bVisible);
    void setLabel(quint32 index, const QString &label);
    void setColor(quint32 index, const QColor &color);
    void setActive(quint32 index, bool bActive);
    void setUnsigned(quint32 index, bool bUnsigned);
    void setMultiplyFactor(quint32 index, double multiplyFactor);
    void setDivideFactor(quint32 index, double divideFactor);
    void setRegisterAddress(quint32 index, const quint16 &registerAddress);
    void setBitmask(quint32 index, const quint16 &bitmask);
    void setShift(quint32 index, const qint32 &shift);

    void add(GraphData rowData);
    void add(QList<GraphData> graphDataList);
    void add();
    void add(QList<QString> labelList, QList<double> timeData, QList<QList<double> > data);

    void removeRegister(qint32 idx);
    void clear();

    void activeGraphAddresList(QList<quint16> * pRegisterList);
    void activeGraphIndexList(QList<quint16> * pList);

    bool areExclusive(quint16 * pRegister, quint16 *pBitmask);

    qint32 convertToActiveGraphIndex(quint32 idx);
    qint32 convertToGraphIndex(quint32 idx);

signals:

    void visibilityChanged(const quint32 index);
    void labelChanged(const quint32 index);
    void colorChanged(const quint32 index);
    void activeChanged(const quint32 index); // when graph is actived / deactivated
    void unsignedChanged(const quint32 index);
    void multiplyFactorChanged(const quint32 index);
    void divideFactorChanged(const quint32 index);
    void registerAddressChanged(const quint32 index);
    void bitmaskChanged(const quint32 index);
    void shiftChanged(const quint32 index);

    void added(const qint32 idx); // When graph definition is added
    void graphsAddData(QList<double>, QList<QList<double> > data);

    void removed(const qint32 idx);
    void cleared();

public slots:

private:
    quint16 nextFreeAddress();

    QList<GraphData> _graphData;
    QList<quint32> _activeGraphList;
};

#endif // GRAPHDATAMODEL_H
