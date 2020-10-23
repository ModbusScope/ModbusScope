#ifndef FORMATDATETIME_H
#define FORMATDATETIME_H

#include <QString>
#include <QDateTime>
#include <QLocale>

namespace FormatDateTime
{

    QString timeStringFormat();
    QString dateStringFormat();
    QString dateTimeStringFormat();

    QString formatDateTime(QDateTime dateTime);
    QString currentDateTime();
}

#endif // FORMATDATETIME_H
