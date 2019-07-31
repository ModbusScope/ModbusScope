#ifndef SETTINGSAUTO_H
#define SETTINGSAUTO_H

#include <QLocale>
#include <QTextStream>

#include "dataparsermodel.h"

class SettingsAuto : public QObject
{
    Q_OBJECT

public:
    explicit SettingsAuto();

    typedef struct
    {
        bool bModbusScopeDataFile;
        QChar fieldSeparator;
        QChar groupSeparator;
        QChar decimalSeparator;
        QString commentSequence;
        quint32 dataRow;
        quint32 column;
        quint32 labelRow;
        bool bTimeInMilliSeconds;
    } settingsData_t;

    bool updateSettings(QTextStream* pDataFileStream, settingsData_t* pSettingsData, qint32 sampleLength);

    static void loadDataFileSample(QTextStream* pDataStream, QStringList* pDataFileSample, qint32 sampleLength);

signals:

public slots:

private:

    bool isAbsoluteDate(QString rawData);
    bool isModbusScopeDataFile(QString firstLine);
    bool isComment(QString line);
    bool testLocale(QStringList previewData, QLocale locale, QChar fieldSeparator);
    quint32 nextDataLine(quint32 startIdx, QStringList previewData, bool *bOk);

    bool _bModbusScopeDataFile;
    QChar _fieldSeparator;
    QChar _groupSeparator;
    QChar _decimalSeparator;
    QString _commentSequence;
    quint32 _dataRow;
    quint32 _column;
    quint32 _labelRow;
    bool _bTimeInMilliSeconds;
};

#endif // SETTINGSAUTO_H
