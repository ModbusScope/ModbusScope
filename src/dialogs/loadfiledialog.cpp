
#include <QtWidgets>
#include <QFileDialog>
#include <QPushButton>
#include <QColor>

#include "settingsauto.h"
#include "util.h"
#include "scopelogging.h"

#include "loadfiledialog.h"
#include "ui_loadfiledialog.h"


const QList<LoadFileDialog::ComboListItem> LoadFileDialog::_fieldSeparatorList
                                    = QList<ComboListItem>() << ComboListItem(" ; (semicolon)", ";")
                                                             << ComboListItem(" , (comma)", ",")
                                                             << ComboListItem(" tab", QString('\t'))
                                                             << ComboListItem(" custom", "custom");

const QList<LoadFileDialog::ComboListItem> LoadFileDialog::_decimalSeparatorList
                                    = QList<ComboListItem>() << ComboListItem(" , (comma)", ",")
                                                            << ComboListItem(" . (point)", ".");

const QList<LoadFileDialog::ComboListItem> LoadFileDialog::_groupSeparatorList
                                    = QList<ComboListItem>() << ComboListItem(" , (comma)", ",")
                                                            << ComboListItem(" . (point)", ".")
                                                            << ComboListItem("   (space)", " ");

const QColor LoadFileDialog::_cColorLabel = QColor(150, 255, 150); // darker screen
const QColor LoadFileDialog::_cColorData = QColor(200, 255, 200); // lighter green
const QColor LoadFileDialog::_cColorIgnored = QColor(175, 175, 175); // grey

const QString LoadFileDialog::_presetFilename = QString("presets.xml");

LoadFileDialog::LoadFileDialog(GuiModel *pGuiModel, DataParserModel * pParserModel, QStringList dataFileSample, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::LoadFileDialog)
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

    // Handle signals from model
    connect(_pParserModel, &DataParserModel::dataFilePathChanged, this, &LoadFileDialog::updatePath);
    connect(_pParserModel, &DataParserModel::fieldSeparatorChanged, this, &LoadFileDialog::updateFieldSeparator);
    connect(_pParserModel, &DataParserModel::groupSeparatorChanged, this, &LoadFileDialog::updategroupSeparator);
    connect(_pParserModel, &DataParserModel::decimalSeparatorChanged, this, &LoadFileDialog::updateDecimalSeparator);
    connect(_pParserModel, &DataParserModel::commentSequenceChanged, this, &LoadFileDialog::updateCommentSequence);
    connect(_pParserModel, &DataParserModel::dataRowChanged, this, &LoadFileDialog::updateDataRow);
    connect(_pParserModel, &DataParserModel::columnChanged, this, &LoadFileDialog::updateColumn);
    connect(_pParserModel, &DataParserModel::labelRowChanged, this, &LoadFileDialog::updateLabelRow);
    connect(_pParserModel, &DataParserModel::timeInMilliSecondsChanged, this, &LoadFileDialog::updateTimeInMilliSeconds);
    connect(_pParserModel, &DataParserModel::stmStudioCorrectionChanged, this, &LoadFileDialog::updateStmStudioCorrection);

    // Handle signal from controls
    connect(_pUi->comboFieldSeparator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LoadFileDialog::fieldSeparatorSelected);
    connect(_pUi->lineCustomFieldSeparator, &QLineEdit::editingFinished, this, &LoadFileDialog::customFieldSeparatorUpdated);
    connect(_pUi->comboGroupSeparator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LoadFileDialog::groupSeparatorSelected);
    connect(_pUi->comboDecimalSeparator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LoadFileDialog::decimalSeparatorSelected);
    connect(_pUi->lineComment, &QLineEdit::editingFinished, this, &LoadFileDialog::commentUpdated);
    connect(_pUi->spinDataRow, QOverload<int>::of(&QSpinBox::valueChanged), this, &LoadFileDialog::dataRowUpdated);
    connect(_pUi->spinColumn, QOverload<int>::of(&QSpinBox::valueChanged), this, &LoadFileDialog::columnUpdated);
    connect(_pUi->spinLabelRow, QOverload<int>::of(&QSpinBox::valueChanged), this, &LoadFileDialog::labelRowUpdated);
    connect(_pUi->comboPreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LoadFileDialog::presetSelected);
    connect(_pUi->checkTimeInMilliSeconds, &QCheckBox::toggled, this, &LoadFileDialog::timeInMilliSecondsUpdated);
    connect(_pUi->checkStmStudioCorrection, &QCheckBox::toggled, this, &LoadFileDialog::stmStudioCorrectionUpdated);

    // Signal to change preset to manual
    connect(_pUi->comboFieldSeparator, QOverload<int>::of(&QComboBox::activated), this, &LoadFileDialog::setPresetToManual);
    connect(_pUi->lineCustomFieldSeparator, &QLineEdit::textEdited, this, &LoadFileDialog::setPresetToManual);
    connect(_pUi->comboGroupSeparator, QOverload<int>::of(&QComboBox::activated), this, &LoadFileDialog::setPresetToManual);
    connect(_pUi->comboDecimalSeparator, QOverload<int>::of(&QComboBox::activated), this, &LoadFileDialog::setPresetToManual);
    connect(_pUi->lineComment, &QLineEdit::textEdited, this, &LoadFileDialog::setPresetToManual);
    connect(_pUi->spinDataRow, &QSpinBox::editingFinished, this, &LoadFileDialog::setPresetToManual);
    connect(_pUi->spinColumn, &QSpinBox::editingFinished, this, &LoadFileDialog::setPresetToManual);
    connect(_pUi->spinLabelRow, &QSpinBox::editingFinished, this, &LoadFileDialog::setPresetToManual);

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

LoadFileDialog::~LoadFileDialog()
{
    delete _pPresetHandler;
    delete _pUi;
}

void LoadFileDialog::updatePath()
{
    _pUi->lineDataFile->setText( QFileInfo(_pParserModel->dataFilePath()).fileName());
}

void LoadFileDialog::updateFieldSeparator()
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

void LoadFileDialog::updategroupSeparator()
{
    _pUi->comboGroupSeparator->setCurrentIndex(findIndexInCombo(_groupSeparatorList, QString(_pParserModel->groupSeparator())));

    updatePreview();
}

void LoadFileDialog::updateDecimalSeparator()
{
    _pUi->comboDecimalSeparator->setCurrentIndex(findIndexInCombo(_decimalSeparatorList, QString(_pParserModel->decimalSeparator())));

    updatePreview();
}

void LoadFileDialog::updateCommentSequence()
{
    _pUi->lineComment->setText(_pParserModel->commentSequence());

    updatePreview();
}

void LoadFileDialog::updateDataRow()
{
    _pUi->spinDataRow->setValue(_pParserModel->dataRow() + 1);  // + 1 based because 0 based internally

    updatePreview();
}

void LoadFileDialog::updateColumn()
{
    _pUi->spinColumn->setValue(_pParserModel->column() + 1);  // + 1 based because 0 based internally

    updatePreview();
}

void LoadFileDialog::updateLabelRow()
{
    _pUi->spinLabelRow->setValue(_pParserModel->labelRow() + 1);   // + 1 based because 0 based internally

    updatePreview();
}

void LoadFileDialog::updateTimeInMilliSeconds()
{
    _pUi->checkTimeInMilliSeconds->setChecked(_pParserModel->timeInMilliSeconds());
}

void LoadFileDialog::updateStmStudioCorrection()
{
    _pUi->checkStmStudioCorrection->setChecked(_pParserModel->stmStudioCorrection());
}

void LoadFileDialog::fieldSeparatorSelected(int index)
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

void LoadFileDialog::customFieldSeparatorUpdated()
{
    if (!_pUi->lineCustomFieldSeparator->text().isEmpty())
    {
        _pParserModel->setFieldSeparator(_pUi->lineCustomFieldSeparator->text().at(0));
    }
}

void LoadFileDialog::groupSeparatorSelected(int index)
{
    if (index > -1)
    {
        _pParserModel->setGroupSeparator(_pUi->comboGroupSeparator->itemData(index).toString().at(0));
    }
}

void LoadFileDialog::decimalSeparatorSelected(int index)
{
    if (index > -1)
    {
        _pParserModel->setDecimalSeparator(_pUi->comboDecimalSeparator->itemData(index).toString().at(0));
    }
}

void LoadFileDialog::commentUpdated()
{
    _pParserModel->setCommentSequence(_pUi->lineComment->text());
}

void LoadFileDialog::dataRowUpdated()
{
    _pParserModel->setDataRow(_pUi->spinDataRow->value() - 1);  // - 1 based because 0 based internally
}

void LoadFileDialog::columnUpdated()
{
    _pParserModel->setColumn(_pUi->spinColumn->value() - 1);  // - 1 based because 0 based internally
}

void LoadFileDialog::labelRowUpdated()
{
    _pParserModel->setLabelRow(_pUi->spinLabelRow->value() - 1);   // - 1 based because 0 based internally
}

void LoadFileDialog::timeInMilliSecondsUpdated(bool bTimeInMilliSeconds)
{
    _pParserModel->setTimeInMilliSeconds(bTimeInMilliSeconds);
}

void LoadFileDialog::stmStudioCorrectionUpdated(bool bCorrectData)
{
    _pParserModel->setStmStudioCorrection(bCorrectData);
}

void LoadFileDialog::presetSelected(int index)
{
    if (index >= static_cast<qint32>(_cPresetListOffset))
    {
        const qint32 presetIndex = index - _cPresetListOffset;

        _pPresetHandler->fillWithPresetData(presetIndex, _pParserModel);
    }
}

void LoadFileDialog::done(int r)
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

void LoadFileDialog::setPresetToManual()
{
    _pUi->comboPreset->setCurrentIndex(_cPresetManualIndex);
}

bool LoadFileDialog::validateSettingsData()
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

qint32 LoadFileDialog::findIndexInCombo(QList<ComboListItem> comboItemList, QString userDataKey)
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

void LoadFileDialog::loadPreset(void)
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

void LoadFileDialog::setPresetAccordingKeyword(QString filename)
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

void LoadFileDialog::updatePreview()
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

void LoadFileDialog::updatePreviewData(QList<QStringList> & previewData)
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

void LoadFileDialog::updatePreviewLayout(bool bValid, QString statusText)
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
