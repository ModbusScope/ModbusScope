#ifndef DATAFILEHANDLER_H
#define DATAFILEHANDLER_H

#include <QObject>

#include "guimodel.h"
#include "graphdatamodel.h"
#include "notemodel.h"
#include "settingsmodel.h"

#include "datafileexporter.h"

class DataFileHandler : public QObject
{
    Q_OBJECT
public:
    explicit DataFileHandler(GuiModel* pGuiModel, GraphDataModel* pGraphDataModel, NoteModel* pNoteModel, SettingsModel *pSettingsModel);
    ~DataFileHandler();

    void loadDataFile(QString dataFilePath);

    void enableExporterDuringLog();
    void disableExporterDuringLog();

    void exportDataFile(QString dataFile);
    bool updateNoteLines(QString dataFile);


signals:

public slots:
    void selectDataImportFile();
    void selectDataExportFile();

    void exportDataLine(double timeData, QList <double> dataValues);
    void rewriteDataFile(void);

private:

    GuiModel* _pGuiModel;
    GraphDataModel* _pGraphDataModel;
    NoteModel* _pNoteModel;
    SettingsModel* _pSettingsModel;

    DataFileExporter * _pDataFileExporter;

};

#endif // DATAFILEHANDLER_H
