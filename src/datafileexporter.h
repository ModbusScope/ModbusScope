#ifndef DATAFILEEXPORTER_H
#define DATAFILEEXPORTER_H

#include <QObject>
#include "guimodel.h"
#include "logmodel.h"
#include "connectionmodel.h"
#include "extendedgraphview.h"

class DataFileExporter : public QObject
{
    Q_OBJECT
public:
    explicit DataFileExporter(GuiModel *pGuiModel, LogModel * pLogModel, ConnectionModel * pConnectionModel, ExtendedGraphView * pGraphView, QObject *parent = 0);
    ~DataFileExporter();

    void exportDataCsv(QString dataFile);

signals:

public slots:

private:
    void writeToFile(QString filePath, QString logData);

    LogModel * _pLogModel;
    GuiModel * _pGuiModel;
    ConnectionModel * _pConnectionModel;
    ExtendedGraphView * _pGraphView;

};

#endif // DATAFILEEXPORTER_H
