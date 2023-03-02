#include "datafileparser.h"
#include "settingsauto.h"
#include "util.h"

#include "datafilehandler.h"
#include "parsedatafiledialog.h"
#include "fileselectionhelper.h"

#include <QWidget>
#include <QProgressDialog>

DataFileHandler::DataFileHandler(GuiModel* pGuiModel, GraphDataModel* pGraphDataModel, NoteModel* pNoteModel, SettingsModel * pSettingsModel, DataParserModel * pDataParserModel, QWidget *parent) : QObject(parent)
{
    _pGuiModel = pGuiModel;
    _pGraphDataModel = pGraphDataModel;
    _pNoteModel = pNoteModel;
    _pSettingsModel = pSettingsModel;
    _pDataParserModel = pDataParserModel;

    _pDataFileStream = nullptr;
    _pDataFile = nullptr;

    _pDataFileExporter = new DataFileExporter(_pGuiModel, _pSettingsModel, _pGraphDataModel, _pNoteModel);

    connect(this, &DataFileHandler::startDataParsing, this, &DataFileHandler::parseDataFile);
}

DataFileHandler::~DataFileHandler()
{
    delete _pDataFileExporter;

    cleanUpFileHandler();
}

void DataFileHandler::openDataFile(QString dataFilePath)
{
    _pDataFile = new QFile(dataFilePath);

    bool bModbusScopeDataFile = false;

    /* Read sample of file */
    if (_pDataFile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        _pDataFileStream = new QTextStream(_pDataFile);
    }
    else
    {
        Util::showError(tr("Couldn't open data file: %1").arg(dataFilePath));

        cleanUpFileHandler();

        /* Stop processing because file is invalid */
        return;
    }

    /* Try to determine settings */
    SettingsAuto autoSettingsParser;
    SettingsAuto::settingsData_t settingsData;

    /* Always set data file name */
    _pDataParserModel->setDataFilePath(dataFilePath);

    bool bRet = autoSettingsParser.updateSettings(_pDataFileStream, &settingsData, _cSampleLineLength);
    if (bRet)
    {
        _pDataParserModel->setFieldSeparator(settingsData.fieldSeparator);
        _pDataParserModel->setGroupSeparator(settingsData.groupSeparator);
        _pDataParserModel->setDecimalSeparator(settingsData.decimalSeparator);
        _pDataParserModel->setCommentSequence(settingsData.commentSequence);
        _pDataParserModel->setDataRow(settingsData.dataRow);
        _pDataParserModel->setColumn(settingsData.column);
        _pDataParserModel->setLabelRow(settingsData.labelRow);
        _pDataParserModel->setTimeInMilliSeconds(settingsData.bTimeInMilliSeconds);

        bModbusScopeDataFile = settingsData.bModbusScopeDataFile;
    }

    if (bRet && bModbusScopeDataFile)
    {
        /* ModbusScope file that can be automatically parsed */
        emit startDataParsing();
    }
    else
    {
        QStringList dataFileSample;
        SettingsAuto::loadDataFileSample(_pDataFileStream, dataFileSample, _cSampleLineLength);

        ParseDataFileDialog parseDataFileDialog(_pGuiModel, _pDataParserModel, dataFileSample);

        if (parseDataFileDialog.exec() == QDialog::Accepted)
        {
            parseDataFile();
        }
        else
        {
            cleanUpFileHandler();
        }
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

bool DataFileHandler::updateNoteLines()
{
    return _pDataFileExporter->updateNoteLines(_pDataParserModel->dataFilePath());
}

void DataFileHandler::selectDataImportFile()
{
    QFileDialog dialog;
    FileSelectionHelper::configureFileDialog(&dialog,
                                             FileSelectionHelper::DIALOG_TYPE_OPEN,
                                             FileSelectionHelper::FILE_TYPE_NONE);
    dialog.setDefaultSuffix("csv");
    dialog.setWindowTitle(tr("Select data file"));

    QStringList extensionFilter = QStringList() << tr("csv file (*.csv)") << tr("any file (*)");
    dialog.setNameFilters(extensionFilter);

    QString selectedFile = FileSelectionHelper::showDialog(&dialog);
    if (!selectedFile.isEmpty())
    {
        this->openDataFile(selectedFile);
    }
}

void DataFileHandler::selectDataExportFile()
{
    QFileDialog dialog;
    FileSelectionHelper::configureFileDialog(&dialog,
                                             FileSelectionHelper::DIALOG_TYPE_SAVE,
                                             FileSelectionHelper::FILE_TYPE_CSV);

    QString selectedFile = FileSelectionHelper::showDialog(&dialog);
    if (!selectedFile.isEmpty())
    {
        _pDataFileExporter->exportDataFile(selectedFile);
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

void DataFileHandler::parseDataFile()
{
    if (_pDataFileStream != nullptr)
    {
        DataFileParser dataParser(_pDataParserModel);
        DataFileParser::FileData data;
        QProgressDialog progressDialog("Loading file...", QString(), 0, 100, dynamic_cast<QWidget *>(parent()));

        progressDialog.setWindowModality(Qt::WindowModal);
        progressDialog.setMinimumDuration(0);

        connect(&dataParser, &DataFileParser::parseErrorOccurred, this, &DataFileHandler::handleError);
        connect(&dataParser, &DataFileParser::updateProgress, &progressDialog, &QProgressDialog::setValue);

        if (dataParser.processDataFile(_pDataFileStream, &data))
        {
            progressDialog.setValue(progressDialog.maximum());

            _pGraphDataModel->clear();
            _pGuiModel->setFrontGraph(-1);

            _pGraphDataModel->add(data.dataLabel);

            if (!data.colors.isEmpty() && data.colors.count() == data.dataLabel.size())
            {
                for (int idx = 0; idx < data.dataLabel.size(); idx++)
                {
                    _pGraphDataModel->setColor(static_cast<quint32>(idx), data.colors[idx]);
                }
            }

            if (!data.axis.isEmpty() && data.axis.count() == data.dataLabel.size())
            {
                for (int idx = 0; idx < data.dataLabel.size(); idx++)
                {
                    auto valueAxis = data.axis[idx] == 1 ? GraphData::VALUE_AXIS_SECONDARY : GraphData::VALUE_AXIS_PRIMARY;
                    _pGraphDataModel->setValueAxis(static_cast<quint32>(idx), valueAxis);
                }
            }

            _pGraphDataModel->setAllData(data.timeRow, data.dataRows);

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
            _pGuiModel->clearMarkersState();
            _pGuiModel->setGuiState(GuiModel::DATA_LOADED);

        }
        else
        {
            progressDialog.cancel();
        }

        cleanUpFileHandler();
    }
}


void DataFileHandler::handleError(QString msg)
{
    Util::showError(msg);
}

void DataFileHandler::cleanUpFileHandler()
{
    if (_pDataFileStream != nullptr)
    {
        delete _pDataFileStream;
        _pDataFileStream = nullptr;
    }

    if (_pDataFile != nullptr)
    {
        delete _pDataFile;
        _pDataFile = nullptr;
    }
}
