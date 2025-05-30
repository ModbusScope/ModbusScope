#ifndef EXPRESSION_CHECKER_H
#define EXPRESSION_CHECKER_H

#include "communication/graphdatahandler.h"
#include "models/graphdatamodel.h"
#include <QObject>

class ExpressionChecker : public QObject
{
    Q_OBJECT
public:

    explicit ExpressionChecker(QObject *parent = nullptr);

    void checkExpression(QString expr);
    QString expression(void);
    void descriptions(QStringList& descr);

    qsizetype requiredValueCount();
    void setValues(ResultDoubleList results);

    bool isValid();
    double result();
    QString strError();
    qint32 errorPos();
    bool syntaxError();

signals:
    void resultsReady(bool valid);

private slots:
    void handleDataReady(ResultDoubleList resultList);

private:

    GraphDataModel _localGraphDataModel;
    GraphDataHandler _graphDataHandler;

    QStringList _descriptions;

    bool _bValid;
    double _result;
    QString _strError;
    qint32 _errorPos;
    bool _bSyntaxError;
};

#endif // EXPRESSION_CHECKER_H
