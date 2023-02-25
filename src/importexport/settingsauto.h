#ifndef SETTINGSAUTO_H
#define SETTINGSAUTO_H

#include <QLocale>
#include <QTextStream>
#include <QRegularExpression>

class SettingsAuto : public QObject
{
    Q_OBJECT

public:
    explicit SettingsAuto();

    typedef struct
    {
        bool bModbusScopeDataFile;
        QString fieldSeparator;
        QString groupSeparator;
        QString decimalSeparator;
        QString commentSequence;
        quint32 dataRow;
        quint32 column;
        quint32 labelRow;
        bool bTimeInMilliSeconds;
    } settingsData_t;

    bool updateSettings(QTextStream* pDataFileStream, settingsData_t* pSettingsData, qint32 sampleLength);

    static void loadDataFileSample(QTextStream* pDataStream, QStringList &dataFileSample, qint32 sampleLength);

private:

    bool isAbsoluteDate(QString rawData);
    bool isModbusScopeDataFile(QString firstLine);
    bool isEmptyLine(QString line);
    bool determineComment(QString line);
    bool testLocale(QStringList previewData, QLocale locale, QString fieldSeparator);
    quint32 nextDataLine(quint32 startIdx, QStringList previewData, bool *bOk);

    bool _bModbusScopeDataFile{};
    QString _fieldSeparator{};
    QString _groupSeparator{};
    QString _decimalSeparator{};
    QString _commentSequence{};
    quint32 _dataRow{};
    quint32 _column{};
    quint32 _labelRow{};
    bool _bTimeInMilliSeconds{};

    QRegularExpression _absoluteDateRegex{};
    static const QString _cAbsoluteDatePattern;
};

#endif // SETTINGSAUTO_H
