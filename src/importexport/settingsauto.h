#ifndef SETTINGSAUTO_H
#define SETTINGSAUTO_H

#include <QLocale>

class SettingsAuto : public QObject
{
    Q_OBJECT

public:
    explicit SettingsAuto();

    bool updateSettings(QStringList previewData);

    QChar fieldSeparator();
    QChar groupSeparator();
    QChar decimalSeparator();
    QString commentSequence();
    quint32 dataRow();
    quint32 column();
    qint32 labelRow();
    bool timeInMilliSeconds();
    QLocale locale();
    bool absoluteDate();

signals:

public slots:

private:

    bool isAbsoluteDate(QString rawData);
    bool isComment(QString line);
    bool testLocale(QStringList previewData, QLocale locale, QChar fieldSeparator);
    quint32 nextDataLine(quint32 startIdx, QStringList previewData, bool *bOk);

    QChar _fieldSeparator;
    QChar _groupSeparator;
    QChar _decimalSeparator;
    QString _commentSequence;
    quint32 _dataRow;
    quint32 _column;
    qint32 _labelRow;
    bool _bAbsoluteDate;
    bool _bTimeInMilliSeconds;
    QLocale _locale;

};

#endif // SETTINGSAUTO_H
