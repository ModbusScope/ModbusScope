#ifndef DATAFILEPARSER_H
#define DATAFILEPARSER_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QRegularExpression>

#include <note.h>
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
        QList<Note> notes;

    } FileData;

    DataFileParser();
    ~DataFileParser();

    bool processDataFile(QString dataFile, FileData * pData);

private:
    bool parseDataLines(QList<QList<double> > &dataRows);
    bool readLineFromFile(QString *pLine);
    void loadDataFileSample(QStringList * pDataFileSample);
    qint64 parseDateTime(QString rawData, bool *bOk);
    bool parseNoteField(QStringList noteFieldList, Note * pNote);


    QTextStream * _pDataStream;

    /* QFile is class variable because it closes file when it goes out-of-scope
     * and read from _pDataStream then gives an SIGSENV error
     */
    QFile * _pDataFile;

    quint32 _expectedFields;

    SettingsAuto * _pAutoSettingsParser;
    QRegularExpression _dateParseRegex;

    static const QString _cPattern;
    static const qint32 _cSampleLineLength = 50;

};

#endif // DATAFILEPARSER_H
