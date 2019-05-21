#include "datafileparser.h"

#include "datafilehandler.h"

DataFileHandler::DataFileHandler(GuiModel* pGuiModel, GraphDataModel* pGraphDataModel, NoteModel* pNoteModel, SettingsModel * pSettingsModel) : QObject(nullptr)
{
    _pGuiModel = pGuiModel;
    _pGraphDataModel = pGraphDataModel;
    _pNoteModel = pNoteModel;
    _pSettingsModel = pSettingsModel;

    _pDataFileExporter = new DataFileExporter(_pGuiModel, _pSettingsModel, _pGraphDataModel, _pNoteModel);

    connect(_pGraphDataModel, SIGNAL(colorChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(activeChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(unsignedChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(multiplyFactorChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(divideFactorChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(registerAddressChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(bitmaskChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(shiftChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(connectionIdChanged(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(added(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
    connect(_pGraphDataModel, SIGNAL(removed(quint32)), _pDataFileExporter, SLOT(rewriteDataFile()));
}

DataFileHandler::~DataFileHandler()
{
    delete _pDataFileExporter;
}

void DataFileHandler::loadDataFile(QString dataFilePath)
{
    DataFileParser dataParser;

    DataFileParser::FileData data;
    if (dataParser.processDataFile(dataFilePath, &data))
    {
        // Set to full auto scaling
        _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO);

        // Set to full auto scaling
        _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO);

        _pGraphDataModel->clear();
        _pGuiModel->setFrontGraph(-1);

        _pGraphDataModel->add(data.dataLabel, data.timeRow, data.dataRows);

        if (!data.colors.isEmpty())
        {
            for (int idx = 0; idx < data.dataLabel.size(); idx++)
            {
                _pGraphDataModel->setColor(static_cast<quint32>(idx), data.colors[idx]);
            }
        }

        _pNoteModel->clear();
        if (!data.notes.isEmpty())
        {
            foreach(Note note, data.notes)
            {
                _pNoteModel->add(note);
            }
        }
        _pNoteModel->setNotesDataUpdated(false);

        _pGuiModel->setFrontGraph(0);

        _pGuiModel->setProjectFilePath("");
        _pGuiModel->setDataFilePath(dataFilePath);

        _pGuiModel->setGuiState(GuiModel::DATA_LOADED);

    }
}

void DataFileHandler::enableExporterDuringLog()
{
    _pDataFileExporter->enableExporterDuringLog();
}

void DataFileHandler::disableExporterDuringLog()
{
    _pDataFileExporter->disableExporterDuringLog();
}

void DataFileHandler::exportDataFile(QString dataFile)
{
    _pDataFileExporter->exportDataFile(dataFile);
}

bool DataFileHandler::updateNoteLines(QString dataFile)
{
    return _pDataFileExporter->updateNoteLines(dataFile);
}

void DataFileHandler::selectDataImportFile()
{
    QString filePath;
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setWindowTitle(tr("Select csv file"));
    dialog.setNameFilter(tr("csv files (*.csv)"));
    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());
        this->loadDataFile(filePath);
    }
}

void DataFileHandler::selectDataExportFile()
{
    QString filePath;
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setDefaultSuffix("csv");
    dialog.setWindowTitle(tr("Select csv file"));
    dialog.setNameFilter(tr("CSV files (*.csv)"));
    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec())
    {
        filePath = dialog.selectedFiles().first();
        _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());
        _pDataFileExporter->exportDataFile(filePath);
    }
}

void DataFileHandler::exportDataLine(double timeData, QList <double> dataValues)
{
    _pDataFileExporter->exportDataLine(timeData, dataValues);
}

void DataFileHandler::rewriteDataFile(void)
{
    _pDataFileExporter->rewriteDataFile();
}
