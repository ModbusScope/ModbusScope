#include "datafileparser.h"
#include "settingsauto.h"
#include "util.h"

#include "datafilehandler.h"
#include "parsedatafiledialog.h"

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

void DataFileHandler::loadDataFile(QString dataFilePath)
{
    // Set last used path
    _pGuiModel->setLastDir(QFileInfo(dataFilePath).dir().absolutePath());

    SettingsAuto * _pAutoSettingsParser;

    _pDataFile = new QFile(dataFilePath);

    bool bModbusScopeDataFile = false;

    /* Read sample of file */
    bool bRet = _pDataFile->open(QIODevice::ReadOnly | QIODevice::Text);
    if (bRet)
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
    if (bRet)
    {
        _pAutoSettingsParser = new SettingsAuto();
        SettingsAuto::settingsData_t settingsData;

        /* Always set data file name */
        _pDataParserModel->setDataFilePath(dataFilePath);

        bRet = _pAutoSettingsParser->updateSettings(_pDataFileStream, &settingsData, _cSampleLineLength);
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
    QString filePath;
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setWindowTitle(tr("Select data file"));

    QStringList extensionFilter = QStringList() << tr("csv file (*.csv)") << tr("any file (*)");
    dialog.setNameFilters(extensionFilter);

    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec())
    {
        auto fileList = dialog.selectedFiles();
        if (!fileList.isEmpty())
        {
            filePath = fileList.at(0);
            this->loadDataFile(filePath);
        }
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
        auto fileList = dialog.selectedFiles();
        if (!fileList.isEmpty())
        {
            filePath = fileList.at(0);
            _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());
            _pDataFileExporter->exportDataFile(filePath);
        }
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

            // Set to full auto scaling
            _pGuiModel->setxAxisScale(AxisMode::SCALE_AUTO);

            // Set to full auto scaling
            _pGuiModel->setyAxisScale(AxisMode::SCALE_AUTO);

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
