
#include "qmuparser.h"

#include "datahandling/muparserregister.h"

#include "muParser.h"

#include <QRegularExpression>

ResultDoubleList QMuParser::_registerValues;

namespace {

/*!
 * \brief Widens a range so it covers every internal register function (r(N)) it overlaps.
 * \param internalExpression The expression as passed to muparser.
 * \param start Start of the range; updated in place.
 * \param end End of the range (exclusive); updated in place.
 */
void widenToRegisterFunctions(const QString& internalExpression, qsizetype& start, qsizetype& end)
{
    static const QRegularExpression registerFunctionRegex(QStringLiteral(R"(\br\(\d+ *\))"));

    QRegularExpressionMatchIterator it = registerFunctionRegex.globalMatch(internalExpression);
    while (it.hasNext())
    {
        const QRegularExpressionMatch match = it.next();
        if (match.capturedStart() < end && match.capturedEnd() > start)
        {
            start = qMin(start, match.capturedStart());
            end = qMax(end, match.capturedEnd());
        }
    }
}

} // namespace

/*!
 * \brief Creates a parser for an expression.
 * \param strExpression The expression in its internal representation, as passed to muparser.
 * \param displayExpression The expression as the user wrote it, used in error messages.
 */
QMuParser::QMuParser(QString strExpression, const QString& displayExpression)
{
    mu::ParserRegister::setRegisterCallback(&QMuParser::registerValue);
    _pExprParser = new mu::ParserRegister();

    _errorPos = -1;
    _errorType = ErrorType::NONE;

    setExpression(strExpression, displayExpression);
}

QMuParser::QMuParser(const QMuParser& source)
    : _pExprParser(new mu::ParserRegister(*source._pExprParser)),
      _displayExpression(source._displayExpression),
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

    _displayExpression = source._displayExpression;
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
 * \param displayExpression The expression as the user wrote it, used in error messages. It is only used when it has
 *        the same length as \a expr, so positions match. Otherwise error messages use \a expr.
 */
void QMuParser::setExpression(QString expr, const QString& displayExpression)
{
    _displayExpression = displayExpression.size() == expr.size() ? displayExpression : QString();

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
 *   Sets the data point values that can be used in the expressions. The data point values are shared
 *   across all instances of QMuParser.
 *
 *   \param regValues A list of Result<double> objects representing the data point values and their validity.
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
        _errorType = ErrorType::SYNTAX;
        /* No error position: the conflict is between two separators, not at one place */
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
                _msg = userFacingMessage(e);
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

/*!
 * \brief Returns the muparser error message with the internal token replaced by the text the user wrote.
 * \param e The exception thrown by muparser.
 * \return The error message to show to the user.
 */
QString QMuParser::userFacingMessage(const mu::ParserError& e) const
{
    QString msg = QString::fromStdWString(e.GetMsg());
    const QString token = QString::fromStdWString(e.GetToken());
    const QString internalExpression = QString::fromStdWString(e.GetExpr());
    qsizetype start = e.GetPos();

    /* Some errors report the position after the token, so only map a token that is found at its position */
    if (_displayExpression.isEmpty() || token.isEmpty() || start < 0 ||
        internalExpression.mid(start, token.size()) != token)
    {
        return msg;
    }

    qsizetype end = start + token.size();
    widenToRegisterFunctions(internalExpression, start, end);

    /* mid() clamps to the end, which drops the trailing space muparser appends to the expression */
    const QString displayToken = _displayExpression.mid(start, end - start);

    msg.replace(QStringLiteral("\"%1\"").arg(token), QStringLiteral("\"%1\"").arg(displayToken));
    return msg;
}

void QMuParser::registerValue(int index, double* value, bool* success)
{
    if (index >= 0 && index < _registerValues.size())
    {
        *value = _registerValues[index].value();
        *success = _registerValues[index].isUsable();
    }
    else
    {
        *value = 0;
        *success = false;
    }
}
