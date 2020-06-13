
#include "qmuparser.h"
#include "muparserregister.h"

#include "muParser.h"

QMuParser::QMuParser(QString strExpression)
{
    _pExprParser = new mu::ParserRegister();
    _pExprParser->SetExpr(strExpression.toStdString());

    reset();
}

QMuParser::~QMuParser()
{
    delete _pExprParser;
}

bool QMuParser::evaluate()
{
    reset();

    try
    {
        _result = _pExprParser->Eval();

        _msg = QStringLiteral("Success");
        _bSuccess = true;
    }
    catch (mu::Parser::exception_type &e)
    {
        _result = 0;

        _msg = QString::fromStdString(e.GetMsg());
        _bSuccess = false;
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
