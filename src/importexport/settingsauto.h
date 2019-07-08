#ifndef SETTINGSAUTO_H
#define SETTINGSAUTO_H

#include <QLocale>

#include "dataparsermodel.h"

class SettingsAuto : public QObject
{
    Q_OBJECT

public:
    explicit SettingsAuto(DataParserModel * pDataParserModel);

    bool updateSettings(QStringList previewData);

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

    DataParserModel *_pDataParserModel;
};

#endif // SETTINGSAUTO_H
