
#include "qmuparser.h"

#include "datahandling/muparserregister.h"

#include "muParser.h"

ResultDoubleList QMuParser::_registerValues;

QMuParser::QMuParser(QString strExpression)
{
    mu::ParserRegister::setRegisterCallback(&QMuParser::registerValue);
    _pExprParser = new mu::ParserRegister();

    _errorPos = -1;
    _errorType = ErrorType::NONE;

    setExpression(strExpression);
}

QMuParser::QMuParser(const QMuParser& source)
    : _pExprParser(new mu::ParserRegister(*source._pExprParser)),
      _bInvalidExpression(source._bInvalidExpression),
      _bSuccess(source._bSuccess),
      _value(source._value),
      _msg(source._msg),
      _errorPos(source._errorPos),
      _errorType(source._errorType)
{
}

QMuParser& QMuParser::operator=(const QMuParser& source)
{
    if (this == &source)
    {
        return *this;
    }

    mu::ParserRegister* newParser = nullptr;
    if (source._pExprParser)
    {
        newParser = new mu::ParserRegister(*source._pExprParser);
    }

    delete _pExprParser;
    _pExprParser = newParser;

    _bInvalidExpression = source._bInvalidExpression;
    _bSuccess = source._bSuccess;
    _value = source._value;
    _msg = source._msg;
    _errorPos = source._errorPos;
    _errorType = source._errorType;

    return *this;
}

QMuParser::~QMuParser()
{
    delete _pExprParser;
}

/*!
 * Sets the expression to be evaluated. If the expression contains both '.' and ',' as decimal separators, it is
 * considered invalid. The method also sets the appropriate decimal separator for muparser based on the content of the
 * expression.
 *
 * \param expr The expression string to be set for evaluation.
 */
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
        _errorPos = -1;
        _errorType = ErrorType::NONE;
    }
    catch (mu::Parser::exception_type& e)
    {
        _bInvalidExpression = false;
        _errorPos = e.GetPos();
        _errorType = ErrorType::SYNTAX;
    }

    reset();
}

/*!
 *   Sets the register values that can be used in the expressions. The register values are shared
 *   across all instances of QMuParser.
 *
 *   \param regValues A list of Result<double> objects representing the register values and their validity.
 */
void QMuParser::setRegistersData(const ResultDoubleList& regValues)
{
    _registerValues = regValues;
}

/*!
 * Evaluates the expression set in the parser. If the expression is invalid due to syntax errors or other issues,
 * it catches the exceptions thrown by muparser and sets the appropriate error messages and types.
 *
 * \return true if the evaluation was successful, false otherwise.
 */
bool QMuParser::evaluate()
{
    reset();

    if (_bInvalidExpression)
    {
        _bSuccess = false;
        _value = 0;
        _msg = QStringLiteral("Invalid expression (unexpected decimal separator)");
        /* Error position already set */
    }
    else
    {
        try
        {
            _value = _pExprParser->Eval();

            if (qIsInf(_value) || qIsNaN(_value))
            {
                throw mu::ParserError(L"Result value is an undefined number. Check input validity.", -1, L"");
            }

            _msg = QStringLiteral("Success");
            _errorPos = -1;
            _errorType = ErrorType::NONE;
            _bSuccess = true;
        }
        catch (mu::Parser::exception_type& e)
        {
            _value = 0;
            _errorPos = e.GetPos();

            const mu::EErrorCodes errCode = e.GetCode();
            if (errCode == mu::ecINTERNAL_ERROR)
            {
                if (_errorPos >= 0 && static_cast<quint64>(_errorPos) <= (e.GetExpr().length() + 1))
                {
                    _msg = QString("Invalid expression (error at position %1)").arg(_errorPos);
                }
                else
                {
                    _msg = QString("Invalid expression");
                    _errorPos = -1;
                }
            }
            else
            {
                _msg = QString::fromStdWString(e.GetMsg());
            }

            _errorType = errCode != mu::ecGENERIC ? ErrorType::SYNTAX : ErrorType::OTHER;

            _bSuccess = false;
        }
    }

    return _bSuccess;
}

QString QMuParser::msg() const
{
    return _msg;
}

qint32 QMuParser::errorPos() const
{
    return _errorPos;
}

QMuParser::ErrorType QMuParser::errorType() const
{
    return _errorType;
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

void QMuParser::registerValue(int index, double* value, bool* success)
{
    if (index >= 0 && index < _registerValues.size())
    {
        *value = _registerValues[index].value();
        *success = _registerValues[index].isValid();
    }
    else
    {
        *value = 0;
        *success = false;
    }
}
