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

private:
    void parseExpressions(const QStringList& expressions);

    QString processExpression(QString const& expr);
    bool processRegisterExpression(const QString& regExpr, DataPoint& dataPoint);
    QString constructInternalRegisterFunction(DataPoint const& dataPoint, int size);

    QStringList _processedExpressions;
    QList<DataPoint> _dataPoints;

    const QRegularExpression _findRegRegex;

    static const QString _cRegisterFunctionTemplate;
};

#endif // EXPRESSIONPARSER_H
