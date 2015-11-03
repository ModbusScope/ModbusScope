#ifndef DATAFILEPARSER_H
#define DATAFILEPARSER_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QFile>

#include "settingsauto.h"

class DataFileParser : public QObject
{
    Q_OBJECT

public:

    typedef struct
    {
        QString axisLabel;
        QList<double> timeRow;

        QStringList dataLabel;
        QList<QList<double> > dataRows;
    } FileData;

    DataFileParser(QFile * pDataFile);
    ~DataFileParser();

    bool processDataFile(FileData * pData);

private:
    bool parseDataLines(QList<QList<double> > &dataRows);
    void showError(QString text);
    bool readLineFromFile(QString *pLine);
    void loadDataFileSample();

    QFile * _pDataFile;
    quint32 _expectedFields;

    SettingsAuto * _pAutoSettingsParser;
    QStringList _dataFileSample;

    static const qint32 _cSampleLineLength = 50;

};

#endif // DATAFILEPARSER_H
