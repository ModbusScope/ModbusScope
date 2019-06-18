#ifndef LoadFileDialog_H
#define LoadFileDialog_H

#include <QDialog>
#include <QStringList>

#include "parsermodel.h"
#include "guimodel.h"
#include "presetparser.h"

namespace Ui {
class LoadFileDialog;
}

class LoadFileDialog : public QDialog
{
    Q_OBJECT

public:

    explicit LoadFileDialog(GuiModel * pGuiModel, ParserModel * pParserModel, QWidget *parent = 0);
    ~LoadFileDialog();

    void open();
    void open(QString file);

private slots:

    void updateDynamicSession();
    void updatePath();
    void updateFieldSeparator();
    void updategroupSeparator();
    void updateDecimalSeparator();
    void updateCommentSequence();
    void updateDataRow();
    void updateColumn();
    void updateLabelRow();
    void updateTimeInMilliSeconds();
    void updateStmStudioCorrection();

    void dynamicSessionUpdated(bool bDynamic);
    void selectDataFile();
    void fieldSeparatorSelected(int index);
    void customFieldSeparatorUpdated();
    void groupSeparatorSelected(int  index);
    void decimalSeparatorSelected(int  index);
    void commentUpdated();
    void dataRowUpdated();
    void columnUpdated();
    void toggledLabelRow(bool bLabelRow);
    void labelRowUpdated();
    void timeInMilliSecondsUpdated(bool bTimeInMilliSeconds);
    void stmStudioCorrectionUpdated(bool bCorrectData);

    void presetSelected(int index);

    void done(int r);
    void setPresetToManual();

private:

    typedef struct _ComboListItem
    {
        _ComboListItem(QString _name, QString _userData)
        {
            name = _name;
            userData = _userData;
        }

        QString name;
        QString userData;
    } ComboListItem;

    Ui::LoadFileDialog * _pUi;

    ParserModel * _pParserModel;
    GuiModel *_pGuiModel;
    PresetParser _presetParser;

    QStringList _dataFileSample;

    bool validateSettingsData();
    qint32 findIndexInCombo(QList<ComboListItem> comboItemList, QString userDataKey);
    void loadPreset(void);
    void setPresetAccordingKeyword(QString filename);
    void updatePreview();
    void updatePreviewData(QList<QStringList> & previewData);
    void updatePreviewLayout();
    void loadDataFileSample();

    static const QList<ComboListItem> _fieldSeparatorList;
    static const QList<ComboListItem> _decimalSeparatorList;
    static const QList<ComboListItem> _groupSeparatorList;
    static const QColor _cColorLabel;
    static const QColor _cColorData;
    static const QColor _cColorIgnored;
    static const qint32 _cSampleLineLength = 50;
    static const quint32 _cPresetManualIndex = 0;
    static const quint32 _cPresetAutoIndex = 1;
    static const quint32 _cPresetListOffset = 2;
};

#endif // LoadFileDialog_H
