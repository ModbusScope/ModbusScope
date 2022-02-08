
#include "qmuparser.h"
#include "muparserregister.h"

#include "muParser.h"

QList<Result<qint64> > QMuParser::_registerValues;

QMuParser::QMuParser(QString strExpression)
{
    mu::ParserRegister::setRegisterCallback(&QMuParser::registerValue);
    _pExprParser = new mu::ParserRegister();

    setExpression(strExpression);
}

QMuParser::QMuParser(const QMuParser &source)
    : _pExprParser(new mu::ParserRegister(*source._pExprParser)),
    _bInvalidExpression(source._bInvalidExpression),
    _bSuccess(source._bSuccess),
    _value(source._value),
    _msg(source._msg)
{

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
        _pExprParser->SetExpr(expr.toStdWString());
    }
    catch (mu::Parser::exception_type &e)
    {
        _bInvalidExpression = false;
    }

    reset();
}

void QMuParser::setRegistersData(QList<Result<qint64> >& regValues)
{
    _registerValues = regValues;
}

QString QMuParser::expression()
{
    return QString::fromStdWString(_pExprParser->GetExpr()).trimmed();
}

bool QMuParser::evaluate()
{
    reset();

    if (_bInvalidExpression)
    {
        _bSuccess = false;
        _value = 0;
        _msg = QStringLiteral("Invalid expression");
    }
    else
    {
        try
        {
            _value = _pExprParser->Eval();

            if (qIsInf(_value) || qIsNaN(_value))
            {
                throw mu::ParserError(L"result is an undefined number");
            }

            _msg = QStringLiteral("Success");
            _bSuccess = true;
        }
        catch (mu::Parser::exception_type &e)
        {
            _value = 0;

            _msg = QString::fromStdWString(e.GetMsg());
            _bSuccess = false;
        }
    }

    return _bSuccess;
}

QString QMuParser::msg() const
{
    return _msg;
}

double QMuParser::value() const
{
    return _value;
}

bool QMuParser::isSuccess() const
{
    return _bSuccess;
}

void QMuParser::reset()
{
    _bSuccess = false;
    _value = 0;
    _msg = QStringLiteral("No result yet");
}

void QMuParser::registerValue(int index, int* value, bool* success)
{
    if (index < _registerValues.size())
    {
        *value = _registerValues[index].value();
        *success = _registerValues[index].isSuccess();
    }
    else
    {
        *value = 0;
        *success = false;
    }
}
