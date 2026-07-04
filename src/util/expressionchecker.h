#ifndef EXPRESSION_CHECKER_H
#define EXPRESSION_CHECKER_H

#include "datahandling/graphdatahandler.h"
#include "models/graphdatamodel.h"
#include <QObject>

class ExpressionChecker : public QObject
{
    Q_OBJECT
public:
    explicit ExpressionChecker(QObject* parent = nullptr);

    void setExpression(const QString& expr);

    QString expression(void);
    QStringList descriptions() const;
    QStringList addresses() const;
    QList<deviceId_t> deviceIds() const;
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

private:
    void handleDataReady(ResultDoubleList resultList);

    GraphDataModel _localGraphDataModel;
    GraphDataHandler _graphDataHandler;

    QList<deviceId_t> _expectedDeviceIdList;

    QStringList _descriptions;
    QStringList _addresses;
    QList<deviceId_t> _deviceIds;

    bool _bValid;
    double _result;
    QString _strError;
    qint32 _errorPos;
    bool _bSyntaxError;
};

#endif // EXPRESSION_CHECKER_H
