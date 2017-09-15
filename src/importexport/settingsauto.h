#ifndef SETTINGSAUTO_H
#define SETTINGSAUTO_H

#include <QObject>
#include <QLocale>
class SettingsAuto : public QObject
{
    Q_OBJECT

public:
    explicit SettingsAuto();

    bool updateSettings(QStringList previewData);

    QChar fieldSeparator();
    QString commentSequence();
    quint32 dataRow();
    qint32 labelRow();
    QLocale locale();
    bool absoluteDate();

signals:

public slots:

private:

    bool isComment(QString line);
    bool isAbsoluteDate(QString rawData);
    bool testLocale(QStringList previewData, QLocale locale, QChar fieldSeparator);
    bool testAbsoluteDate(QStringList previewData);
    quint32 nextDataLine(quint32 startIdx, QStringList previewData, bool *bOk);


    QChar _fieldSeparator;
    QString _commentSequence;
    quint32 _dataRow;
    qint32 _labelRow;
    bool _bAbsoluteDate;
    QLocale _locale;

};

#endif // SETTINGSAUTO_H
