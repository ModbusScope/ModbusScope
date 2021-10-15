
#include "formatdatetime.h"

QString FormatDateTime::timeStringFormat()
{
    return QString("HH:mm:ss%1zzz").arg(QLocale().decimalPoint());
}

QString FormatDateTime::dateStringFormat()
{
    return QString("dd/MM/yyyy");
}

QString FormatDateTime::dateTimeStringFormat()
{
    return QString("%1 %2").arg(dateStringFormat(), timeStringFormat());
}

QString FormatDateTime::formatDateTime(QDateTime dateTime)
{
    return dateTime.toString(dateTimeStringFormat());
}

QString FormatDateTime::currentDateTime()
{
    return formatDateTime(QDateTime::currentDateTime());
}
