#ifndef DATAFILEEXPORTER_H
#define DATAFILEEXPORTER_H

#include <QObject>
#include <QStringList>

/* Forward declaration */
class SettingsModel;
class GuiModel;
class GraphDataModel;
class NoteModel;

class DataFileExporter : public QObject
{
    Q_OBJECT
public:
    explicit DataFileExporter(GuiModel *pGuiModel, SettingsModel *pSettingsModel, GraphDataModel * pGraphDataModel, NoteModel * pNoteModel, QObject *parent = 0);
    ~DataFileExporter();

    void enableExporterDuringLog();
    void disableExporterDuringLog();

    void exportDataFile(QString dataFile);
    bool updateNoteLines(QString dataFile);

signals:

public slots:
    void exportDataLine(double timeData, QList <double> dataValues);
    void rewriteDataFile(void);

private:

    typedef enum
    {
        E_LABEL = 0u,
        E_PROPERTY,
        E_COLOR,
        E_EXPRESSION,
        E_VALUE_AXIS,

    } registerProperty;

    void flushExportBuffer();
    void exportDataHeader();
    QStringList constructDataHeader(bool bDuringLog);
    QString constructConnSettings(quint8 connectionId);
    void createNoteRows(QStringList& noteRows);
    QString createPropertyRow(registerProperty prop);
    QString formatData(double timeData, QList<double> dataValues);
    bool writeToFile(QString filePath, QStringList logData);
    void clearFile(QString filePath);

    GuiModel * _pGuiModel;
    SettingsModel * _pSettingsModel;
    GraphDataModel * _pGraphDataModel;
    NoteModel * _pNoteModel;

    QStringList _dataExportBuffer;
    quint64 lastLogTime;

    static const quint64 _cLogBufferTimeout = 1000; /* in milliseconds */
    static const quint32 _cLogChunkLineCount = 1000;

};

#endif // DATAFILEEXPORTER_H
