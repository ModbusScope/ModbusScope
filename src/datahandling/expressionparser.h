#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include "communication/datapoint.h"
#include <QRegularExpression>
#include <QStringList>

class ExpressionParser : public QObject
{
    Q_OBJECT
public:
    explicit ExpressionParser(QStringList& expressions);

    void dataPoints(QList<DataPoint>& dataPointList);
    void processedExpressions(QStringList& expressionList);

private:
    void parseExpressions(QStringList& expressions);

    bool parseAddress(QString strAddr, DataPoint& dataPoint);
    bool parseDeviceId(QString strDeviceId, DataPoint& dataPoint);
    bool parseType(QString strType, DataPoint& dataPoint);

    QString processExpression(QString const& expr);
    bool processRegisterExpression(QString regExpr, DataPoint& dataPoint);
    QString constructInternalRegisterFunction(DataPoint const& dataPoint, int size);

    QStringList _processedExpressions;
    QList<DataPoint> _dataPoints;

    QRegularExpression _findRegRegex;
    QRegularExpression _regParseRegex;

    static const QString _cRegisterFunctionTemplate;
};

#endif // EXPRESSIONPARSER_H
