#ifndef DATAFILEPARSER_H
#define DATAFILEPARSER_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QFile>
#include <QTextStream>
#include <QStringList>

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
        QList<QColor> colors;
    } FileData;

    DataFileParser();
    ~DataFileParser();

    bool processDataFile(QString dataFile, FileData * pData);

private:
    bool parseDataLines(QList<QList<double> > &dataRows);
    void showError(QString text);
    bool readLineFromFile(QString *pLine);
    void loadDataFileSample(QStringList * pDataFileSample);

    QTextStream * _pDataStream;

    /* QFile is class variable because it closes file when it goes out-of-scope
     * and read from _pDataStream then gives an SIGSENV error
     */
    QFile * _pDataFile;

    quint32 _expectedFields;

    SettingsAuto * _pAutoSettingsParser;

    static const qint32 _cSampleLineLength = 50;

};

#endif // DATAFILEPARSER_H
