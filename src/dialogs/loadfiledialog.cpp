
#include <QFileDialog>
#include <QColor>

#include "settingsauto.h"
#include "util.h"

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


LoadFileDialog::LoadFileDialog(GuiModel *pGuiModel, DataParserModel * pParserModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::LoadFileDialog)
{
    _pUi->setupUi(this);

    _pParserModel = pParserModel;
    _pGuiModel = pGuiModel;

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
    connect(_pParserModel, SIGNAL(dataFilePathChanged()), this, SLOT(updatePath()));
    connect(_pParserModel, SIGNAL(fieldSeparatorChanged()), this, SLOT(updateFieldSeparator()));
    connect(_pParserModel, SIGNAL(groupSeparatorChanged()), this, SLOT(updategroupSeparator()));
    connect(_pParserModel, SIGNAL(decimalSeparatorChanged()), this, SLOT(updateDecimalSeparator()));
    connect(_pParserModel, SIGNAL(commentSequenceChanged()), this, SLOT(updateCommentSequence()));
    connect(_pParserModel, SIGNAL(dataRowChanged()), this, SLOT(updateDataRow()));
    connect(_pParserModel, SIGNAL(columnChanged()), this, SLOT(updateColumn()));
    connect(_pParserModel, SIGNAL(labelRowChanged()), this, SLOT(updateLabelRow()));
    connect(_pParserModel, SIGNAL(timeInMilliSecondsChanged()), this, SLOT(updateTimeInMilliSeconds()));
    connect(_pParserModel, SIGNAL(stmStudioCorrectionChanged()), this, SLOT(updateStmStudioCorrection()));


    // Handle signal from controls
    connect(_pUi->comboFieldSeparator, SIGNAL(currentIndexChanged(int)), this, SLOT(fieldSeparatorSelected(int)));
    connect(_pUi->lineCustomFieldSeparator, SIGNAL(editingFinished()), this, SLOT(customFieldSeparatorUpdated()));
    connect(_pUi->comboGroupSeparator, SIGNAL(currentIndexChanged(int)), this, SLOT(groupSeparatorSelected(int)));
    connect(_pUi->comboDecimalSeparator, SIGNAL(currentIndexChanged(int)), this, SLOT(decimalSeparatorSelected(int)));
    connect(_pUi->lineComment, SIGNAL(editingFinished()), this, SLOT(commentUpdated()));
    connect(_pUi->spinDataRow, SIGNAL(valueChanged(int)), this, SLOT(dataRowUpdated()));
    connect(_pUi->spinColumn, SIGNAL(valueChanged(int)), this, SLOT(columnUpdated()));
    connect(_pUi->spinLabelRow, SIGNAL(valueChanged(int)), this, SLOT(labelRowUpdated()));
    connect(_pUi->comboPreset, SIGNAL(currentIndexChanged(int)), this, SLOT(presetSelected(int)));
    connect(_pUi->checkTimeInMilliSeconds, SIGNAL(toggled(bool)), this, SLOT(timeInMilliSecondsUpdated(bool)));
    connect(_pUi->checkStmStudioCorrection, SIGNAL(toggled(bool)), this, SLOT(stmStudioCorrectionUpdated(bool)));

    // Signal to change preset to manual
    connect(_pUi->comboFieldSeparator, SIGNAL(activated(int)), this, SLOT(setPresetToManual()));
    connect(_pUi->lineCustomFieldSeparator, SIGNAL(textEdited(QString)), this, SLOT(setPresetToManual()));
    connect(_pUi->comboGroupSeparator, SIGNAL(activated(int)), this, SLOT(setPresetToManual()));
    connect(_pUi->comboDecimalSeparator, SIGNAL(activated(int)), this, SLOT(setPresetToManual()));
    connect(_pUi->lineComment, SIGNAL(textEdited(QString)), this, SLOT(setPresetToManual()));
    connect(_pUi->spinDataRow, SIGNAL(editingFinished()), this, SLOT(setPresetToManual()));
    connect(_pUi->spinColumn, SIGNAL(editingFinished()), this, SLOT(setPresetToManual()));
    connect(_pUi->spinLabelRow, SIGNAL(editingFinished()), this, SLOT(setPresetToManual()));

    // Select first preset
    _pUi->comboPreset->setCurrentIndex(-1);
    _pUi->comboPreset->setCurrentIndex(0);

    _pParserModel->triggerUpdate();
}

LoadFileDialog::~LoadFileDialog()
{
    delete _pUi;
}

void LoadFileDialog::open()
{
    /* Shouldn't be used */
    _dataFileSample.clear();

    QDialog::open();
}

void LoadFileDialog::open(QTextStream* pDataStream, qint32 sampleLineLength)
{
    SettingsAuto::loadDataFileSample(pDataStream, &_dataFileSample, sampleLineLength);

    setPresetAccordingKeyword(_pParserModel->dataFilePath());

    updatePreview();

    QDialog::open();
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
    const qint32 presetIndex = index - _cPresetListOffset;

    if ((presetIndex >= 0) && (presetIndex < _presetParser.presetList().size()))
    {
        _pParserModel->setColumn(_presetParser.presetList()[presetIndex].column -1);
        _pParserModel->setDataRow(_presetParser.presetList()[presetIndex].dataRow - 1);
        _pParserModel->setLabelRow(_presetParser.presetList()[presetIndex].labelRow - 1);
        _pParserModel->setDecimalSeparator(_presetParser.presetList()[presetIndex].decimalSeparator);
        _pParserModel->setFieldSeparator(_presetParser.presetList()[presetIndex].fieldSeparator);
        _pParserModel->setGroupSeparator(_presetParser.presetList()[presetIndex].thousandSeparator);
        _pParserModel->setCommentSequence(_presetParser.presetList()[presetIndex].commentSequence);
        _pParserModel->setTimeInMilliSeconds(_presetParser.presetList()[presetIndex].bTimeInMilliSeconds);
        _pParserModel->setStmStudioCorrection(_presetParser.presetList()[presetIndex].bStmStudioCorrection);
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
        if (!QFileInfo(_pParserModel->dataFilePath()).exists())
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
    _presetParser.loadPresetsFromFile();

    _pUi->comboPreset->clear();
    _pUi->comboPreset->addItem("Manual");

    foreach(PresetParser::Preset preset, _presetParser.presetList())
    {
        _pUi->comboPreset->addItem(preset.name);
    }
}

void LoadFileDialog::setPresetAccordingKeyword(QString filename)
{
    qint32 presetComboIndex = -1;

    // Loop through presets and set preset if keyword is in filename
    for (qint32 index = 0; index < _presetParser.presetList().size(); index ++)
    {
        if (!_presetParser.presetList()[index].keyword.isEmpty())
        {
            if (QFileInfo(filename).fileName().contains(_presetParser.presetList()[index].keyword, Qt::CaseInsensitive))
            {
                presetComboIndex = index + _cPresetListOffset;
                break;
            }
        }
    }

    // No preset found
    if (presetComboIndex == -1)
    {
        // set to manual
        presetComboIndex = _cPresetManualIndex;
    }

    _pUi->comboPreset->setCurrentIndex(-1);
    _pUi->comboPreset->setCurrentIndex(presetComboIndex);
}

void LoadFileDialog::updatePreview()
{
    bool bRet = false;
    QList<QStringList> previewData;

    if (
        (_pParserModel->fieldSeparator() == _pParserModel->groupSeparator())
        || (_pParserModel->decimalSeparator() == _pParserModel->groupSeparator())
        || (_pParserModel->decimalSeparator() == _pParserModel->fieldSeparator())
        )
    {
        previewData.append(QStringList(tr("Parser setting are not valid (field, group, decimal)!")));
    }
    else if (_pParserModel->labelRow() >= _pParserModel->dataRow())
    {
        previewData.append(QStringList(tr("Label row is greater data row!")));
    }
    else if (_dataFileSample.size() == 0)
    {
        previewData.append(QStringList(tr("No data file loaded!")));
    }
    else if (_dataFileSample.size() < static_cast<qint32>(_pParserModel->labelRow()))
    {
        previewData.append(QStringList(tr("No labels according to label row!")));
    }
    else if (_dataFileSample.size() < static_cast<qint32>(_pParserModel->dataRow() + 1))
    {
        previewData.append(QStringList(tr("No data according to data row!")));
    }
    else
    {
        for (qint32 idx = 0; idx < _dataFileSample.size(); idx++)
        {
            previewData.append(_dataFileSample[idx].trimmed().split(_pParserModel->fieldSeparator()));
        }

        bRet = true;
    }

    // Add data to table preview component
    updatePreviewData(previewData);

    if (bRet)
    {
        // Update layout of table preview component
        updatePreviewLayout();
    }

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

void LoadFileDialog::updatePreviewLayout()
{
    for (qint32 rowIdx = 0; rowIdx < _pUi->tablePreview->rowCount(); rowIdx++)
    {
        for (qint32 columnIdx = 0; columnIdx < _pUi->tablePreview->columnCount(); columnIdx++)
        {
            if (columnIdx >= static_cast<qint32>(_pParserModel->column()))
            {
                if (rowIdx == static_cast<qint32>(_pParserModel->labelRow()))
                {
                    _pUi->tablePreview->item(rowIdx, columnIdx)->setBackgroundColor(_cColorLabel);
                }
                else if (rowIdx >= static_cast<qint32>(_pParserModel->dataRow()))
                {
                    _pUi->tablePreview->item(rowIdx, columnIdx)->setBackgroundColor(_cColorData);
                }
                else
                {
                    _pUi->tablePreview->item(rowIdx, columnIdx)->setTextColor(_cColorIgnored);
                }
            }
        }
    }
}
