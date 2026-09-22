#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include "communication/datapoint.h"
#include <QRegularExpression>
#include <QStringList>

class ExpressionParser
{
public:
    explicit ExpressionParser(const QStringList& expressions);

    QList<DataPoint> dataPoints() const;
    QStringList processedExpressions() const;
    QList<QList<int>> expressionDataPointIndices() const;

private:
    void parseExpressions(const QStringList& expressions);

    QString processExpression(QString const& expr, QList<int>& dataPointIndices);
    bool processRegisterExpression(const QString& regExpr, DataPoint& dataPoint);
    int findOrAddDataPoint(DataPoint const& dataPoint);
    QString constructInternalRegisterFunction(int idx, int size);

    QStringList _processedExpressions;
    QList<DataPoint> _dataPoints;
    QList<QList<int>> _expressionDataPointIndices;

    QRegularExpression _findRegRegex;

    static const QString _cRegisterFunctionTemplate;
};

#endif // EXPRESSIONPARSER_H
