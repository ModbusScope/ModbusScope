#ifndef EXPRESSION_CHECKER_H
#define EXPRESSION_CHECKER_H

#include "datahandling/graphdatahandler.h"
#include "models/graphdatamodel.h"
#include <QObject>

class ExpressionChecker : public QObject
{
    Q_OBJECT
public:

    explicit ExpressionChecker(QObject *parent = nullptr);

    void setExpression(QString expr);

    QString expression(void);
    void descriptions(QStringList& descr);
    qsizetype requiredValueCount();

    bool checkForDevices(QList<deviceId_t> const& deviceIdList);
    void checkWithValues(ResultDoubleList results);

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

    QList<deviceId_t> _expectedDeviceIdList;

    QStringList _descriptions;

    bool _bValid;
    double _result;
    QString _strError;
    qint32 _errorPos;
    bool _bSyntaxError;
};

#endif // EXPRESSION_CHECKER_H
