#ifndef DATAFILEHANDLER_H
#define DATAFILEHANDLER_H

#include <QFile>
#include <QObject>

// Forward declaration
class GuiModel;
class GraphDataModel;
class NoteModel;
class SettingsModel;
class DataParserModel;
class DataFileExporter;

class DataFileHandler : public QObject
{
    Q_OBJECT
public:
    explicit DataFileHandler(GuiModel* pGuiModel, GraphDataModel* pGraphDataModel, NoteModel* pNoteModel, SettingsModel *pSettingsModel, DataParserModel * pDataParserModel, QWidget *parent = nullptr);
    ~DataFileHandler();

    void openDataFile(QString dataFilePath);

    void enableExporterDuringLog();
    void disableExporterDuringLog();

    void exportDataFile(QString dataFile);
    bool updateNoteLines();

signals:
    void startDataParsing();

public slots:
    void selectDataImportFile();
    void selectDataExportFile();

    void exportDataLine(double timeData, QList <double> dataValues);
    void rewriteDataFile(void);

    void parseDataFile();

private slots:
    void handleError(QString msg);
    void cleanUpFileHandler();

private:

    GuiModel* _pGuiModel;
    GraphDataModel* _pGraphDataModel;
    NoteModel* _pNoteModel;
    SettingsModel* _pSettingsModel;

    DataFileExporter * _pDataFileExporter;
    DataParserModel * _pDataParserModel;

    QTextStream* _pDataFileStream;
    QFile* _pDataFile;

    static const qint32 _cSampleLineLength = 50;
};

#endif // DATAFILEHANDLER_H
