/*!
 * \brief Header file for the QMuParser class, which provides an interface to the muparser library for evaluating
 * mathematical expressions.
 *
 * The QMuParser class is a wrapper around muparser that provides an interface to set the expression,
 * evaluate it, and retrieve the result and error information. It also allows setting register values
 * that can be used in the expression. The class handles syntax errors and other exceptions thrown by
 * muparser and provides error messages and positions for easier debugging.
 *
 * All QMuParser instances share the same register values, which are set through the static method
 * setRegistersData.
 */
#ifndef QMUPARSER_H
#define QMUPARSER_H

#include <QObject>

#include "datahandling/muparserregister.h"
#include "util/result.h"

class QMuParser
{

public:
    enum class ErrorType
    {
        NONE = 0,
        SYNTAX,
        OTHER,
    };

    explicit QMuParser(QString strExpression);
    QMuParser(const QMuParser& source);

    QMuParser& operator=(const QMuParser& source);
    ~QMuParser();

    void setExpression(QString expr);

    static void setRegistersData(const ResultDoubleList& regValues);

    bool evaluate();

    bool isSuccess() const;
    QString msg() const;
    qint32 errorPos() const;
    ErrorType errorType() const;
    double value() const;

private:
    void reset();

    static void registerValue(int index, double* value, bool* success);

    static ResultDoubleList _registerValues;

    mu::ParserRegister* _pExprParser;

    bool _bInvalidExpression;

    bool _bSuccess;
    double _value;
    QString _msg;
    qint32 _errorPos;
    ErrorType _errorType;
};

#endif // QMUPARSER_H
