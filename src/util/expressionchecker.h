#ifndef EXPRESSION_CHECKER_H
#define EXPRESSION_CHECKER_H

#include <QObject>
#include "graphdatamodel.h"
#include "graphdatahandler.h"

class ExpressionChecker : public QObject
{
    Q_OBJECT
public:

    explicit ExpressionChecker(QObject *parent = nullptr);

    void checkExpression(QString expr);
    QString expression(void);
    void descriptions(QStringList& descr);

    quint32 requiredValueCount();
    void setValues(ResultDoubleList results);

    bool isValid();
    double result();
    QString strError();
    qint32 errorPos();

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

};

#endif // EXPRESSION_CHECKER_H
