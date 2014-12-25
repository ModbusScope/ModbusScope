#ifndef DATAFILEPARSER_H
#define DATAFILEPARSER_H

#include "QObject"
#include "QStringList"
#include "QList"
#include "QFile"

class DataFileParser : public QObject
{
    Q_OBJECT

public:

    typedef struct
    {
        QStringList dataLabel;
        QStringList dataRegister;
        QList<QList<qint64> > dataRows;

    } FileData;

    DataFileParser(QFile * pDataFile);

    bool processDataFile(FileData * pData);

private:
    bool parseDataLines(QList<QList<qint64> > &dataRows);
    void showError(QString text);
    bool readLineFromFile(QString *pLine);

    QFile * _pDataFile;
    quint32 _expectedFields;

};

#endif // DATAFILEPARSER_H
