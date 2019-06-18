#include <QColor>
#include <QLocale>
#include "parsermodel.h"

ParserModel::ParserModel(QObject *parent) : QObject(parent)
{
    _path = "";

    resetSettings();
}

ParserModel::~ParserModel()
{

}

void ParserModel::resetSettings()
{
    /*-- Set default item --*/
    const QChar decimalPoint = QLocale::system().decimalPoint();

    // DecimalSeparator
    if (decimalPoint == '.')
    {
        _decimalSeparator = '.';
    }
    else
    {
        _decimalSeparator = ',';
    }

    // FieldSeparator
    if (decimalPoint == ',')
    {
        _fieldSeparator = ';';
    }
    else
    {
        _fieldSeparator = ',';
    }

    _groupSeparator = QLocale::system().groupSeparator();
    _commentSequence = "";
    _bDynamicSession = false;
    _dataRow = 1;
    _column = 0;
    _labelRow = 0;
    _bTimeInMilliSeconds = true;
    _bStmStudioCorrection = false;
}

void ParserModel::triggerUpdate(void)
{
    emit dynamicSessionChanged();
    emit pathChanged();
    emit fieldSeparatorChanged();
    emit groupSeparatorChanged();
    emit decimalSeparatorChanged();
    emit commentSequenceChanged();
    emit dataRowChanged();
    emit columnChanged();
    emit labelRowChanged();
    emit timeInMilliSecondsChanged();
    emit stmStudioCorrectionChanged();
}

QString ParserModel::path()
{
    return _path;
}

bool ParserModel::dynamicSession() const
{
    return _bDynamicSession;
}


void ParserModel::setPath(QString path)
{
    if (_path != path.trimmed())
    {
        _path = path.trimmed();
        emit pathChanged();
    }
}

void ParserModel::setDynamicSession(bool bDynamicSession)
{
    if (_bDynamicSession != bDynamicSession)
    {
        _bDynamicSession = bDynamicSession;
        emit dynamicSessionChanged();
    }
}

void ParserModel::setFieldSeparator(QChar fieldSeparator)
{
    // Exception to the rule, always generate event when field separator is changed
    // This is needed for these custom field separator
    //if (_fieldSeparator != fieldSeparator)
    {
        _fieldSeparator = fieldSeparator;
        emit fieldSeparatorChanged();
    }
}

void ParserModel::setGroupSeparator(QChar groupSeparator)
{
    if (_groupSeparator != groupSeparator)
    {
        _groupSeparator = groupSeparator;
        emit groupSeparatorChanged();
    }
}

void ParserModel::setDecimalSeparator(QChar decimalSeparator)
{
    if (_decimalSeparator != decimalSeparator)
    {
        _decimalSeparator = decimalSeparator;
        emit decimalSeparatorChanged();
    }
}

void ParserModel::setCommentSequence(QString commentSequence)
{
    if (_commentSequence != commentSequence.trimmed())
    {
        _commentSequence = commentSequence.trimmed();
        emit commentSequenceChanged();
    }
}

void ParserModel::setDataRow(quint32 dataRow)
{
    if (_dataRow != dataRow)
    {
        _dataRow = dataRow;
        emit dataRowChanged();
    }
}

void ParserModel::setColumn(quint32 column)
{
    if (_column != column)
    {
        _column = column;
        emit columnChanged();
    }
}

void ParserModel::setLabelRow(qint32 labelRow)
{
    qint32 newLabelRow;
    if (labelRow >= 0)
    {
        newLabelRow = labelRow;
    }
    else
    {
        newLabelRow = -1;
    }

    if (_labelRow != newLabelRow)
    {
        _labelRow = newLabelRow;
        emit labelRowChanged();
    }
}

void ParserModel::setTimeInMilliSeconds(bool timeInMilliSeconds)
{
    if (_bTimeInMilliSeconds != timeInMilliSeconds)
    {
        _bTimeInMilliSeconds = timeInMilliSeconds;
        emit timeInMilliSecondsChanged();
    }
}

void ParserModel::setStmStudioCorrection(bool stmStudioCorrection)
{
    if (_bStmStudioCorrection != stmStudioCorrection)
    {
        _bStmStudioCorrection = stmStudioCorrection;
        emit stmStudioCorrectionChanged();
    }
}

QChar ParserModel::fieldSeparator() const
{
    return _fieldSeparator;
}

QChar ParserModel::groupSeparator() const
{
    return _groupSeparator;
}

QChar ParserModel::decimalSeparator() const
{
    return _decimalSeparator;
}

QString ParserModel::commentSequence() const
{
    return _commentSequence;
}

quint32 ParserModel::dataRow() const
{
    return _dataRow;
}

quint32 ParserModel::column() const
{
    return _column;
}

qint32 ParserModel::labelRow() const
{
    return _labelRow;
}

bool ParserModel::timeInMilliSeconds() const
{
    return _bTimeInMilliSeconds;
}

bool ParserModel::stmStudioCorrection() const
{
    return _bStmStudioCorrection;
}

