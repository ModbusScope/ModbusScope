#include "dataparsermodel.h"

#include <QColor>
#include <QLocale>

DataParserModel::DataParserModel(QObject *parent) : QObject(parent)
{
    resetSettings();
}

DataParserModel::~DataParserModel()
{

}

void DataParserModel::resetSettings()
{
    /*-- Set default item --*/
    const QString decimalPoint = QString(QLocale().decimalPoint());

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

    _groupSeparator = QLocale().groupSeparator();
    _commentSequence = "";
    _dataRow = 1;
    _column = 0;
    _labelRow = 0;
    _bTimeInMilliSeconds = true;
    _dataFilePath = "";    
}

void DataParserModel::triggerUpdate(void)
{
    emit fieldSeparatorChanged();
    emit groupSeparatorChanged();
    emit decimalSeparatorChanged();
    emit commentSequenceChanged();
    emit dataRowChanged();
    emit columnChanged();
    emit labelRowChanged();
    emit timeInMilliSecondsChanged();
    emit dataFilePathChanged();
}

void DataParserModel::setFieldSeparator(QString fieldSeparator)
{
    // Exception to the rule, always generate event when field separator is changed
    // This is needed for these custom field separator
    //if (_fieldSeparator != fieldSeparator)
    {
        _fieldSeparator = fieldSeparator;
        emit fieldSeparatorChanged();
    }
}

void DataParserModel::setGroupSeparator(QString groupSeparator)
{
    if (_groupSeparator != groupSeparator)
    {
        _groupSeparator = groupSeparator;
        emit groupSeparatorChanged();
    }
}

void DataParserModel::setDecimalSeparator(QString decimalSeparator)
{
    if (_decimalSeparator != decimalSeparator)
    {
        _decimalSeparator = decimalSeparator;
        emit decimalSeparatorChanged();
    }
}

void DataParserModel::setCommentSequence(QString commentSequence)
{
    if (_commentSequence != commentSequence.trimmed())
    {
        _commentSequence = commentSequence.trimmed();
        emit commentSequenceChanged();
    }
}

void DataParserModel::setDataRow(quint32 dataRow)
{
    if (_dataRow != dataRow)
    {
        _dataRow = dataRow;
        emit dataRowChanged();
    }
}

void DataParserModel::setColumn(quint32 column)
{
    if (_column != column)
    {
        _column = column;
        emit columnChanged();
    }
}

void DataParserModel::setLabelRow(quint32 labelRow)
{
    if (_labelRow != labelRow)
    {
        _labelRow = labelRow;
        emit labelRowChanged();
    }
}

void DataParserModel::setTimeInMilliSeconds(bool timeInMilliSeconds)
{
    if (_bTimeInMilliSeconds != timeInMilliSeconds)
    {
        _bTimeInMilliSeconds = timeInMilliSeconds;
        emit timeInMilliSecondsChanged();
    }
}

void DataParserModel::setDataFilePath(QString path)
{
    if (_dataFilePath != path)
    {
        _dataFilePath = path;
        emit dataFilePathChanged();
    }
}

QString DataParserModel::fieldSeparator() const
{
    return _fieldSeparator;
}

QString DataParserModel::groupSeparator() const
{
    return _groupSeparator;
}

QString DataParserModel::decimalSeparator() const
{
    return _decimalSeparator;
}

QString DataParserModel::commentSequence() const
{
    return _commentSequence;
}

quint32 DataParserModel::dataRow() const
{
    return _dataRow;
}

quint32 DataParserModel::column() const
{
    return _column;
}

quint32 DataParserModel::labelRow() const
{
    return _labelRow;
}

bool DataParserModel::timeInMilliSeconds() const
{
    return _bTimeInMilliSeconds;
}

QString DataParserModel::dataFilePath()
{
    return _dataFilePath;
}
