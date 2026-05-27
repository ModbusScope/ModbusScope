
#include "tst_formatdatetime.h"

#include "util/formatdatetime.h"

#include <QTest>
#include <QDateTime>
#include <QLocale>

void TestFormatDateTime::init()
{
    QLocale::setDefault(QLocale(QLocale::Dutch, QLocale::Belgium));
}

void TestFormatDateTime::cleanup()
{
}

void TestFormatDateTime::dateStringFormat()
{
    QCOMPARE(FormatDateTime::dateStringFormat(), QStringLiteral("dd/MM/yyyy"));
}

void TestFormatDateTime::timeStringFormat()
{
    QCOMPARE(FormatDateTime::timeStringFormat(), QStringLiteral("HH:mm:ss,zzz"));

    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    QCOMPARE(FormatDateTime::timeStringFormat(), QStringLiteral("HH:mm:ss.zzz"));
}

void TestFormatDateTime::dateTimeStringFormat()
{
    QCOMPARE(FormatDateTime::dateTimeStringFormat(), QStringLiteral("dd/MM/yyyy HH:mm:ss,zzz"));
}

void TestFormatDateTime::formatDateTime()
{
    // Construct a local-time QDateTime directly to avoid timezone conversion
    const QDateTime dt = QDateTime(QDate(2020, 10, 17), QTime(11, 57, 5, 11), Qt::LocalTime);

    QCOMPARE(FormatDateTime::formatDateTime(dt), QStringLiteral("17/10/2020 11:57:05,011"));
}

void TestFormatDateTime::currentDateTime()
{
    const QString result = FormatDateTime::currentDateTime();

    // Round-trip: the formatted string must parse back to a valid QDateTime
    const QDateTime parsed = QDateTime::fromString(result, FormatDateTime::dateTimeStringFormat());
    QVERIFY(parsed.isValid());
}

QTEST_GUILESS_MAIN(TestFormatDateTime)
