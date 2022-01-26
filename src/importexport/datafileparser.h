#ifndef DATAFILEPARSER_H
#define DATAFILEPARSER_H

#include <QTextStream>
#include <QRegularExpression>

#include "note.h"
#include "dataparsermodel.h"

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

    DataFileParser(DataParserModel * pDataParserModel);
    ~DataFileParser();

    bool processDataFile(QTextStream * pDataStream, FileData * pData);

signals:
    void parseErrorOccurred(QString msg);
    void updateProgress(int percentage);

private:
    bool parseDataLines(QTextStream *pDataStream, QList<QList<double> > &dataRows);
    bool readLineFromFile(QTextStream *pDataStream, QString *pLine);
    qint64 parseDateTime(QString rawData, bool *bOk);
    bool parseNoteField(QStringList noteFieldList, Note * pNote);
    double parseDouble(QString strNumber, bool* bOk);
    bool isCommentLine(QString line);
    void checkProgressUpdate(quint32 charRead);

    void correctStmStudioData(QList<QList<double> > &dataLists);
    bool isNibbleCorrupt(quint16 ref, quint16 compare);

    quint32 _lineNumber;

    quint64 _totalCharSize;
    quint64 _charCount;
    quint32 _lastPercentageUpdate;

    DataParserModel* _pDataParserModel;

    quint32 _expectedFields;

    QRegularExpression _dateParseRegex;

    static const QString _cPattern;

};

#endif // DATAFILEPARSER_H
