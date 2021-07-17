
#include <QtWidgets>
#include <QFileDialog>
#include <QPushButton>
#include <QColor>

#include "settingsauto.h"
#include "util.h"
#include "scopelogging.h"

#include "parsedatafiledialog.h"
#include "ui_parsedatafiledialog.h"


const QList<ParseDataFileDialog::ComboListItem> ParseDataFileDialog::_fieldSeparatorList
                                    = QList<ComboListItem>() << ComboListItem(" ; (semicolon)", ";")
                                                             << ComboListItem(" , (comma)", ",")
                                                             << ComboListItem(" tab", QString('\t'))
                                                             << ComboListItem(" custom", "custom");

const QList<ParseDataFileDialog::ComboListItem> ParseDataFileDialog::_decimalSeparatorList
                                    = QList<ComboListItem>() << ComboListItem(" , (comma)", ",")
                                                            << ComboListItem(" . (point)", ".");

const QList<ParseDataFileDialog::ComboListItem> ParseDataFileDialog::_groupSeparatorList
                                    = QList<ComboListItem>() << ComboListItem(" , (comma)", ",")
                                                            << ComboListItem(" . (point)", ".")
                                                            << ComboListItem("   (space)", " ");

const QColor ParseDataFileDialog::_cColorLabel = QColor(150, 255, 150); // darker screen
const QColor ParseDataFileDialog::_cColorData = QColor(200, 255, 200); // lighter green
const QColor ParseDataFileDialog::_cColorIgnored = QColor(175, 175, 175); // grey

const QString ParseDataFileDialog::_presetFilename = QString("presets.xml");

ParseDataFileDialog::ParseDataFileDialog(GuiModel *pGuiModel, DataParserModel * pParserModel, QStringList dataFileSample, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ParseDataFileDialog)
{
    _pUi->setupUi(this);

    _pParserModel = pParserModel;
    _pGuiModel = pGuiModel;
    _dataFileSample = dataFileSample;

    _pPresetHandler = new PresetHandler(new PresetParser());

    // load presets
    loadPreset();

    _pUi->tablePreview->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _pUi->tablePreview->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    _pUi->tablePreview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _pUi->tablePreview->setFocusPolicy(Qt::NoFocus);

    /*-- Fill combo boxes --*/
    foreach(ComboListItem listItem, _decimalSeparatorList)
    {
        _pUi->comboDecimalSeparator->addItem(listItem.name, listItem.userData);
    }

    foreach(ComboListItem listItem, _fieldSeparatorList)
    {
        _pUi->comboFieldSeparator->addItem(listItem.name, listItem.userData);
    }

    foreach(ComboListItem listItem, _groupSeparatorList)
    {
        _pUi->comboGroupSeparator->addItem(listItem.name, listItem.userData);
    }

    _pTimeFormatGroup = new QButtonGroup();
    _pTimeFormatGroup->setExclusive(true);
    _pTimeFormatGroup->addButton(_pUi->radioSeconds, 0);
    _pTimeFormatGroup->addButton(_pUi->radioMilliSeconds, 1);
    connect(_pTimeFormatGroup, &QButtonGroup::idClicked, this, &ParseDataFileDialog::timeFormatUpdated);

    // Handle signals from model
    connect(_pParserModel, &DataParserModel::dataFilePathChanged, this, &ParseDataFileDialog::updatePath);
    connect(_pParserModel, &DataParserModel::fieldSeparatorChanged, this, &ParseDataFileDialog::updateFieldSeparator);
    connect(_pParserModel, &DataParserModel::groupSeparatorChanged, this, &ParseDataFileDialog::updategroupSeparator);
    connect(_pParserModel, &DataParserModel::decimalSeparatorChanged, this, &ParseDataFileDialog::updateDecimalSeparator);
    connect(_pParserModel, &DataParserModel::commentSequenceChanged, this, &ParseDataFileDialog::updateCommentSequence);
    connect(_pParserModel, &DataParserModel::dataRowChanged, this, &ParseDataFileDialog::updateDataRow);
    connect(_pParserModel, &DataParserModel::columnChanged, this, &ParseDataFileDialog::updateColumn);
    connect(_pParserModel, &DataParserModel::labelRowChanged, this, &ParseDataFileDialog::updateLabelRow);
    connect(_pParserModel, &DataParserModel::timeInMilliSecondsChanged, this, &ParseDataFileDialog::updateTimeFormat);
    connect(_pParserModel, &DataParserModel::stmStudioCorrectionChanged, this, &ParseDataFileDialog::updateStmStudioCorrection);

    // Handle signal from controls
    connect(_pUi->comboFieldSeparator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ParseDataFileDialog::fieldSeparatorSelected);
    connect(_pUi->lineCustomFieldSeparator, &QLineEdit::editingFinished, this, &ParseDataFileDialog::customFieldSeparatorUpdated);
    connect(_pUi->comboGroupSeparator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ParseDataFileDialog::groupSeparatorSelected);
    connect(_pUi->comboDecimalSeparator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ParseDataFileDialog::decimalSeparatorSelected);
    connect(_pUi->lineComment, &QLineEdit::editingFinished, this, &ParseDataFileDialog::commentUpdated);
    connect(_pUi->spinDataRow, QOverload<int>::of(&QSpinBox::valueChanged), this, &ParseDataFileDialog::dataRowUpdated);
    connect(_pUi->spinColumn, QOverload<int>::of(&QSpinBox::valueChanged), this, &ParseDataFileDialog::columnUpdated);
    connect(_pUi->spinLabelRow, QOverload<int>::of(&QSpinBox::valueChanged), this, &ParseDataFileDialog::labelRowUpdated);
    connect(_pUi->comboPreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ParseDataFileDialog::presetSelected);
    connect(_pUi->checkStmStudioCorrection, &QCheckBox::toggled, this, &ParseDataFileDialog::stmStudioCorrectionUpdated);

    // Signal to change preset to manual
    connect(_pUi->comboFieldSeparator, QOverload<int>::of(&QComboBox::activated), this, &ParseDataFileDialog::setPresetToManual);
    connect(_pUi->lineCustomFieldSeparator, &QLineEdit::textEdited, this, &ParseDataFileDialog::setPresetToManual);
    connect(_pUi->comboGroupSeparator, QOverload<int>::of(&QComboBox::activated), this, &ParseDataFileDialog::setPresetToManual);
    connect(_pUi->comboDecimalSeparator, QOverload<int>::of(&QComboBox::activated), this, &ParseDataFileDialog::setPresetToManual);
    connect(_pUi->lineComment, &QLineEdit::textEdited, this, &ParseDataFileDialog::setPresetToManual);
    connect(_pUi->spinDataRow, &QSpinBox::editingFinished, this, &ParseDataFileDialog::setPresetToManual);
    connect(_pUi->spinColumn, &QSpinBox::editingFinished, this, &ParseDataFileDialog::setPresetToManual);
    connect(_pUi->spinLabelRow, &QSpinBox::editingFinished, this, &ParseDataFileDialog::setPresetToManual);

    // Select first preset
    _pUi->comboPreset->setCurrentIndex(-1);
    _pUi->comboPreset->setCurrentIndex(0);

    _pUi->lblImportStatus->setText(QString());
    QPalette palette = _pUi->lblImportStatus->palette();
    palette.setColor(_pUi->lblImportStatus->foregroundRole(), Qt::red);
    _pUi->lblImportStatus->setPalette(palette);

    _pParserModel->triggerUpdate();

    setPresetAccordingKeyword(_pParserModel->dataFilePath());

    updatePreview();
}

ParseDataFileDialog::~ParseDataFileDialog()
{
    delete _pPresetHandler;
    delete _pUi;
}

void ParseDataFileDialog::updatePath()
{
    _pUi->lineDataFile->setText( QFileInfo(_pParserModel->dataFilePath()).fileName());
}

void ParseDataFileDialog::updateFieldSeparator()
{
    const qint32 comboIndex = findIndexInCombo(_fieldSeparatorList, QString(_pParserModel->fieldSeparator()));
    const qint32 customIndex = findIndexInCombo(_fieldSeparatorList, tr("custom"));

    if (comboIndex == -1)
    {
        // Custom field seperator
        if (_pUi->comboFieldSeparator->currentIndex() != customIndex)
        {
            _pUi->comboFieldSeparator->setCurrentIndex(customIndex);
        }

        _pUi->lineCustomFieldSeparator->setText(QString(_pParserModel->fieldSeparator()));
    }
    else
    {
        _pUi->comboFieldSeparator->setCurrentIndex(comboIndex);
        _pUi->lineCustomFieldSeparator->setText(QString(""));
    }

    updatePreview();
}

void ParseDataFileDialog::updategroupSeparator()
{
    _pUi->comboGroupSeparator->setCurrentIndex(findIndexInCombo(_groupSeparatorList, QString(_pParserModel->groupSeparator())));

    updatePreview();
}

void ParseDataFileDialog::updateDecimalSeparator()
{
    _pUi->comboDecimalSeparator->setCurrentIndex(findIndexInCombo(_decimalSeparatorList, QString(_pParserModel->decimalSeparator())));

    updatePreview();
}

void ParseDataFileDialog::updateCommentSequence()
{
    _pUi->lineComment->setText(_pParserModel->commentSequence());

    updatePreview();
}

void ParseDataFileDialog::updateDataRow()
{
    _pUi->spinDataRow->setValue(_pParserModel->dataRow() + 1);  // + 1 based because 0 based internally

    updatePreview();
}

void ParseDataFileDialog::updateColumn()
{
    _pUi->spinColumn->setValue(_pParserModel->column() + 1);  // + 1 based because 0 based internally

    updatePreview();
}

void ParseDataFileDialog::updateLabelRow()
{
    _pUi->spinLabelRow->setValue(_pParserModel->labelRow() + 1);   // + 1 based because 0 based internally

    updatePreview();
}

void ParseDataFileDialog::updateTimeFormat()
{
    if (_pParserModel->timeInMilliSeconds())
    {
        _pUi->radioMilliSeconds->setChecked(true);
    }
    else
    {
        _pUi->radioSeconds->setChecked(true);
    }
}

void ParseDataFileDialog::updateStmStudioCorrection()
{
    _pUi->checkStmStudioCorrection->setChecked(_pParserModel->stmStudioCorrection());
}

void ParseDataFileDialog::fieldSeparatorSelected(int index)
{
    if (index > -1)
    {
        if (_pUi->comboFieldSeparator->itemData(index).toString().toLower() == tr("custom"))
        {
            // Enable custom field box
            _pUi->lineCustomFieldSeparator->setEnabled(true);
        }
        else
        {
            // Disable custom field box
            _pUi->lineCustomFieldSeparator->setEnabled(false);

            _pParserModel->setFieldSeparator(_pUi->comboFieldSeparator->itemData(index).toString().at(0));
        }
    }
}

void ParseDataFileDialog::customFieldSeparatorUpdated()
{
    if (!_pUi->lineCustomFieldSeparator->text().isEmpty())
    {
        _pParserModel->setFieldSeparator(_pUi->lineCustomFieldSeparator->text().at(0));
    }
}

void ParseDataFileDialog::groupSeparatorSelected(int index)
{
    if (index > -1)
    {
        _pParserModel->setGroupSeparator(_pUi->comboGroupSeparator->itemData(index).toString().at(0));
    }
}

void ParseDataFileDialog::decimalSeparatorSelected(int index)
{
    if (index > -1)
    {
        _pParserModel->setDecimalSeparator(_pUi->comboDecimalSeparator->itemData(index).toString().at(0));
    }
}

void ParseDataFileDialog::commentUpdated()
{
    _pParserModel->setCommentSequence(_pUi->lineComment->text());
}

void ParseDataFileDialog::dataRowUpdated()
{
    _pParserModel->setDataRow(_pUi->spinDataRow->value() - 1);  // - 1 based because 0 based internally
}

void ParseDataFileDialog::columnUpdated()
{
    _pParserModel->setColumn(_pUi->spinColumn->value() - 1);  // - 1 based because 0 based internally
}

void ParseDataFileDialog::labelRowUpdated()
{
    _pParserModel->setLabelRow(_pUi->spinLabelRow->value() - 1);   // - 1 based because 0 based internally
}

void ParseDataFileDialog::timeFormatUpdated(int id)
{
    _pParserModel->setTimeInMilliSeconds(id ? true: false);
}

void ParseDataFileDialog::stmStudioCorrectionUpdated(bool bCorrectData)
{
    _pParserModel->setStmStudioCorrection(bCorrectData);
}

void ParseDataFileDialog::presetSelected(int index)
{
    if (index >= static_cast<qint32>(_cPresetListOffset))
    {
        const qint32 presetIndex = index - _cPresetListOffset;

        _pPresetHandler->fillWithPresetData(presetIndex, _pParserModel);
    }
}

void ParseDataFileDialog::done(int r)
{
    bool bValid = true;

    if(QDialog::Accepted == r)  // ok was pressed
    {
        // Validate the data
        bValid = validateSettingsData();
    }
    else
    {
        // cancel, close or exc was pressed;
        bValid = true;
    }

    if (bValid)
    {
        QDialog::done(r);
    }
}

void ParseDataFileDialog::setPresetToManual()
{
    _pUi->comboPreset->setCurrentIndex(_cPresetManualIndex);
}

bool ParseDataFileDialog::validateSettingsData()
{
    bool bOk = true;

    if (bOk)
    {
        if (!QFileInfo::exists(_pParserModel->dataFilePath()))
        {
            bOk = false;
            Util::showError(tr("Data file doesn't exist"));
        }
    }

    if (bOk)
    {
        if (_pParserModel->fieldSeparator().isNull())
        {
            bOk = false;
            Util::showError(tr("Custom field separator isn't defined correctly"));
        }
    }

    return bOk;
}

qint32 ParseDataFileDialog::findIndexInCombo(QList<ComboListItem> comboItemList, QString userDataKey)
{
    qint32 index = -1;

    for (qint32 i = 0; i < comboItemList.size(); i++)
    {
        if (comboItemList[i].userData.compare(userDataKey, Qt::CaseInsensitive) == 0)
        {
            index = i;
            break;
        }
    }

    return index;
}

void ParseDataFileDialog::loadPreset(void)
{
    QString presetFile;
    PresetHandler::determinePresetFile(presetFile);

    _pPresetHandler->loadPresetsFromFile(presetFile);

    _pUi->comboPreset->clear();
    _pUi->comboPreset->addItem("Manual");

    foreach(auto name, _pPresetHandler->nameList())
    {
        _pUi->comboPreset->addItem(name);
    }
}

void ParseDataFileDialog::setPresetAccordingKeyword(QString filename)
{
    qint32 presetComboIndex;
    const qint32 presetIdx = _pPresetHandler->determinePreset(filename);

    if (presetIdx == -1)
    {
        // No matching preset found
        presetComboIndex = _cPresetManualIndex;
    }
    else
    {
         presetComboIndex = presetIdx + _cPresetListOffset;
    }

    _pUi->comboPreset->setCurrentIndex(-1);
    _pUi->comboPreset->setCurrentIndex(presetComboIndex);
}

void ParseDataFileDialog::updatePreview()
{
    bool bRet = false;
    QList<QStringList> previewData;
    QString stateText;

    previewData.clear();
    stateText.clear();

    if (
        (_pParserModel->fieldSeparator() == _pParserModel->groupSeparator())
        || (_pParserModel->decimalSeparator() == _pParserModel->groupSeparator())
        || (_pParserModel->decimalSeparator() == _pParserModel->fieldSeparator())
        )
    {
        stateText = tr("Parser setting are not valid (field, group, decimal)!");
    }
    else if (_pParserModel->labelRow() >= _pParserModel->dataRow())
    {
        stateText = tr("Label row is greater data row!");
    }
    else if (_dataFileSample.size() == 0)
    {
        stateText = tr("No data file loaded!");
    }
    else if (_dataFileSample.size() < static_cast<qint32>(_pParserModel->labelRow()))
    {
        stateText = tr("No labels according to label row!");
    }
    else if (_dataFileSample.size() < static_cast<qint32>(_pParserModel->dataRow() + 1))
    {
        stateText = tr("No data according to data row!");
    }
    else
    {
        stateText.clear();

        bRet = true;
    }

    for (qint32 idx = 0; idx < _dataFileSample.size(); idx++)
    {
        QStringList lineData;
        if (bRet)
        {
            lineData = _dataFileSample[idx].trimmed().split(_pParserModel->fieldSeparator());
        }
        else
        {
            lineData = QStringList() << _dataFileSample[idx].trimmed();
        }

        previewData.append(lineData);
    }

    // Add data to table preview component
    updatePreviewData(previewData);

    // Update layout of table preview component
    updatePreviewLayout(bRet, stateText);
}

void ParseDataFileDialog::updatePreviewData(QList<QStringList> & previewData)
{
    // find maximum of columns
    qint32 maxCol = 0;
    foreach (QStringList col, previewData)
    {
        if (col.size() > maxCol)
        {
            maxCol = col.size();
        }
    }

    // Clear data
    _pUi->tablePreview->clear();

    // Set size (column and row)
    _pUi->tablePreview->setColumnCount(maxCol);
    _pUi->tablePreview->setRowCount(previewData.size());

    // Add data
    for(qint32 rowIdx = 0; rowIdx < _pUi->tablePreview->rowCount(); rowIdx++)
    {
        for(qint32 columnIdx = 0; columnIdx < _pUi->tablePreview->columnCount(); columnIdx++)
        {
            QStringList rowData = previewData[rowIdx];
            QString cellData = QString("");
            if (columnIdx < rowData.size())
            {
                cellData = rowData[columnIdx];
            }
            _pUi->tablePreview->setItem(rowIdx, columnIdx, new QTableWidgetItem(cellData));
        }
    }
}

void ParseDataFileDialog::updatePreviewLayout(bool bValid, QString statusText)
{
    if (bValid)
    {
        _pUi->btnBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        _pUi->lblImportStatus->setText(QString());

        for (qint32 rowIdx = 0; rowIdx < _pUi->tablePreview->rowCount(); rowIdx++)
        {
            for (qint32 columnIdx = 0; columnIdx < _pUi->tablePreview->columnCount(); columnIdx++)
            {
                if (columnIdx >= static_cast<qint32>(_pParserModel->column()))
                {
                    if (rowIdx == static_cast<qint32>(_pParserModel->labelRow()))
                    {
                        _pUi->tablePreview->item(rowIdx, columnIdx)->setBackground(_cColorLabel);
                    }
                    else if (rowIdx >= static_cast<qint32>(_pParserModel->dataRow()))
                    {
                        _pUi->tablePreview->item(rowIdx, columnIdx)->setBackground(_cColorData);
                    }
                    else
                    {
                        _pUi->tablePreview->item(rowIdx, columnIdx)->setForeground(_cColorIgnored);
                    }
                }
            }
        }
    }
    else
    {
        _pUi->btnBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        _pUi->lblImportStatus->setText(statusText);

        for (qint32 rowIdx = 0; rowIdx < _pUi->tablePreview->rowCount(); rowIdx++)
        {
            for (qint32 columnIdx = 0; columnIdx < _pUi->tablePreview->columnCount(); columnIdx++)
            {
                _pUi->tablePreview->item(rowIdx, columnIdx)->setForeground(_cColorIgnored);
            }
        }
    }
}
