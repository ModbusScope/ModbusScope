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

    void enableExporterDuringLog();
    void disableExporterDuringLog();

    void exportDataFile(QString dataFile);

signals:

public slots:
    void exportDataLine(double timeData, QList <double> dataValues);

private:
    void flushExportBuffer();
    void exportDataHeader();
    QString constructDataHeader(bool bDuringLog);
    QString createLabelRow();
    QString formatData(double timeData, QList<double> dataValues);
    void writeToFile(QString filePath, QStringList logData);
    void clearFile(QString filePath);

    LogModel * _pLogModel;
    GuiModel * _pGuiModel;
    ConnectionModel * _pConnectionModel;
    ExtendedGraphView * _pGraphView;

    QStringList _dataExportBuffer;
    quint64 lastLogTime;
    bool _bExportDuringLog;

    static const quint64 _cLogBufferTimeout = 1000; /* in milliseconds */

};

#endif // DATAFILEEXPORTER_H
