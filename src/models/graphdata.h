#ifndef GRAPHDATA_H
#define GRAPHDATA_H

#include <QColor>
#include "qcustomplot.h"

class GraphData
{

public:
    explicit GraphData();
    ~GraphData();

    bool isVisible() const;
    void setVisible(bool bVisible);

    QString label() const;
    void setLabel(const QString &label);

    QColor color() const;
    void setColor(const QColor &color);

    bool isActive() const;
    void setActive(bool bActive);

    bool isUnsigned() const;
    void setUnsigned(bool bUnsigned);

    double multiplyFactor() const;
    void setMultiplyFactor(double multiplyFactor);

    double divideFactor() const;
    void setDivideFactor(double divideFactor);

    quint16 registerAddress() const;
    void setRegisterAddress(const quint16 &registerAddress);

    quint16 bitmask() const;
    void setBitmask(const quint16 &bitmask);

    qint32 shift() const;
    void setShift(const qint32 &shift);

    QSharedPointer<QCPGraphDataContainer> dataMap();

private:

    bool _bVisible;
    QString _label;
    QColor _color;

    bool _bActive;
    bool _bUnsigned;
    double _multiplyFactor;
    double _divideFactor;
    quint16 _registerAddress;
    quint16 _bitmask;
    qint32 _shift;

    QSharedPointer<QCPGraphDataContainer> _pDataMap;

};

#endif // GRAPHDATA_H
