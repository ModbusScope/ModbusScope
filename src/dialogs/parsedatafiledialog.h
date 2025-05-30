#ifndef ParseDataFileDialog_H
#define ParseDataFileDialog_H

#include "importexport/presethandler.h"
#include "models/dataparsermodel.h"

#include <QButtonGroup>
#include <QDialog>
#include <QStringList>

namespace Ui {
class ParseDataFileDialog;
}

class ParseDataFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParseDataFileDialog(DataParserModel* pParserModel, QStringList dataFileSample, QWidget* parent = nullptr);
    ~ParseDataFileDialog();

private slots:

    void updatePath();
    void updateFieldSeparator();
    void updategroupSeparator();
    void updateDecimalSeparator();
    void updateCommentSequence();
    void updateDataRow();
    void updateColumn();
    void updateLabelRow();
    void updateTimeFormat();

    void fieldSeparatorSelected(int index);
    void customFieldSeparatorUpdated();
    void groupSeparatorSelected(int  index);
    void decimalSeparatorSelected(int  index);
    void commentUpdated();
    void dataRowUpdated();
    void columnUpdated();
    void labelRowUpdated();
    void timeFormatUpdated(int id);

    void presetSelected(int index);

    void done(int r);
    void setPresetToManual();

private:

    typedef struct _ComboListItem
    {
        _ComboListItem(QString _name, QString _userData): name(_name), userData(_userData)
        {
        }

        QString name;
        QString userData;
    } ComboListItem;

    Ui::ParseDataFileDialog * _pUi;

    DataParserModel * _pParserModel;

    PresetHandler* _pPresetHandler;
    QButtonGroup* _pTimeFormatGroup;

    QStringList _dataFileSample;

    bool validateSettingsData();
    qint32 findIndexInCombo(QList<ComboListItem> comboItemList, QString userDataKey);
    void loadPreset(void);
    void setPresetAccordingKeyword(QString filename);
    void updatePreview();
    void updatePreviewData(QList<QStringList> & previewData);
    void updatePreviewLayout(bool bValid, QString statusText);

    static const QList<ComboListItem> _fieldSeparatorList;
    static const QList<ComboListItem> _decimalSeparatorList;
    static const QList<ComboListItem> _groupSeparatorList;
    static const QColor _cColorLabel;
    static const QColor _cColorData;
    static const QColor _cColorIgnored;
    static const quint32 _cPresetManualIndex = 0;
    static const quint32 _cPresetListOffset = 1;
};

#endif // ParseDataFileDialog_H
