
#include "qmuparser.h"
#include "muparserregister.h"

#include "muParser.h"

QList<double> QMuParser::_registerValues;

QMuParser::QMuParser(QString strExpression)
{
    mu::ParserRegister::setRegisterCallback(&QMuParser::registerValue);
    _pExprParser = new mu::ParserRegister();

    setExpression(strExpression);
}

QMuParser::~QMuParser()
{
    delete _pExprParser;
}

void QMuParser::setExpression(QString expr)
{
    /* Fixed by design */
    _pExprParser->SetArgSep(static_cast<mu::char_type>(';'));

    const bool bContainsDecimalPoint = expr.contains('.');
    const bool bContainsComma = expr.contains(',');

    if (bContainsDecimalPoint && bContainsComma)
    {
        _bInvalidExpression = true;
    }
    else
    {
        _bInvalidExpression = false;

        if (bContainsDecimalPoint)
        {
            _pExprParser->SetDecSep('.');
        }
        else if (bContainsComma)
        {
            _pExprParser->SetDecSep(',');
        }
        else
        {
            /* Keep current locale settings */
            /* Locale settings are static (shared across all instances) */
        }
    }

    try
    {
        _pExprParser->SetExpr(expr.toStdString());
    }
    catch (mu::Parser::exception_type &e)
    {
        _bInvalidExpression = false;
    }

    reset();
}

void QMuParser::setRegisterValues(QList<double>& regValues)
{
    _registerValues = regValues;
}

QString QMuParser::expression()
{
    return QString::fromStdString(_pExprParser->GetExpr()).trimmed();
}

bool QMuParser::evaluate()
{
    reset();

    if (_bInvalidExpression)
    {
        _bSuccess = false;
        _result = 0;
        _msg = QStringLiteral("Invalid expression");
    }
    else
    {
        try
        {
            _result = _pExprParser->Eval();

            if (qIsInf(_result) || qIsNaN(_result))
            {
                throw mu::ParserError("result is an undefined number");
            }

            _msg = QStringLiteral("Success");
            _bSuccess = true;
        }
        catch (mu::Parser::exception_type &e)
        {
            _result = 0;

            _msg = QString::fromStdString(e.GetMsg());
            _bSuccess = false;
        }
    }

    return _bSuccess;
}

QString QMuParser::msg()
{
    return _msg;
}

double QMuParser::result()
{
    return _result;
}

bool QMuParser::isSuccess()
{
    return _bSuccess;
}

void QMuParser::reset()
{
    _bSuccess = false;
    _result = 0;
    _msg = QStringLiteral("No result yet");
}

int QMuParser::registerValue(int index)
{
    if (index < _registerValues.size())
    {
        return _registerValues[index];
    }
    else
    {
        return 0;
    }
}
