#include "datafileparser.h"
#include "settingsauto.h"
#include "util.h"

#include "datafilehandler.h"

DataFileHandler::DataFileHandler(GuiModel* pGuiModel, GraphDataModel* pGraphDataModel, NoteModel* pNoteModel, SettingsModel * pSettingsModel, DataParserModel * pDataParserModel) : QObject(nullptr)
{
    _pGuiModel = pGuiModel;
    _pGraphDataModel = pGraphDataModel;
    _pNoteModel = pNoteModel;
    _pSettingsModel = pSettingsModel;
    _pDataParserModel = pDataParserModel;

    _pDataFileStream = nullptr;

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

    connect(this, &DataFileHandler::startDataParsing, this, &DataFileHandler::parseDataFile);

    _pLoadFileDialog = new LoadFileDialog(pGuiModel, pDataParserModel);

    connect(_pLoadFileDialog, &LoadFileDialog::accepted, this, &DataFileHandler::parseDataFile);
}

DataFileHandler::~DataFileHandler()
{
    delete _pLoadFileDialog;
    delete _pDataFileExporter;
    delete _pDataParserModel;

    if (_pDataFileStream != nullptr)
    {
        delete _pDataFileStream;
    }
}

void DataFileHandler::loadDataFile(QString dataFilePath)
{
    // Set last used path
    _pGuiModel->setLastDir(QFileInfo(dataFilePath).dir().absolutePath());

    SettingsAuto * _pAutoSettingsParser;

    QFile* dataFile = new QFile(dataFilePath);

    bool bModbusScopeDataFile = false;

    /* Read sample of file */
    bool bRet = dataFile->open(QIODevice::ReadOnly | QIODevice::Text);
    if (bRet)
    {
        _pDataFileStream = new QTextStream(dataFile);
    }
    else
    {
        Util::showError(tr("Couldn't open data file: %1").arg(dataFilePath));

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
        else
        {
            Util::showError(tr("Invalid data file (error while auto parsing for settings)"));
        }
    }

    if (bRet && bModbusScopeDataFile)
    {
        /* ModbusScope file that can be automatically parsed */
        emit startDataParsing();
    }
    else
    {
        /* Open load file dialog */
        _pLoadFileDialog->open(_pDataFileStream, _cSampleLineLength);
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
        filePath = dialog.selectedFiles().first();
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

void DataFileHandler::parseDataFile()
{
    if (_pDataFileStream != nullptr)
    {
        DataFileParser dataParser(_pDataParserModel);
        DataFileParser::FileData data;

        connect(&dataParser, &DataFileParser::parseErrorOccurred, this, &DataFileHandler::handleError);

        if (dataParser.processDataFile(_pDataFileStream, &data))
        {
            delete _pDataFileStream;
            _pDataFileStream = nullptr;

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

            _pGuiModel->setGuiState(GuiModel::DATA_LOADED);

        }
    }
}


void DataFileHandler::handleError(QString msg)
{
    Util::showError(msg);
}
