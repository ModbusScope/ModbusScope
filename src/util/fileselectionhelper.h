#ifndef FILESELECTIONHELPER_H
#define FILESELECTIONHELPER_H

#include <QObject>

class QFileDialog;
class GuiModel;

class FileSelectionHelper : public QObject
{
    Q_OBJECT
public:

    typedef enum
    {
        DIALOG_TYPE_OPEN,
        DIALOG_TYPE_SAVE,
    } DialogType;

    typedef enum
    {
        FILE_TYPE_PNG,
        FILE_TYPE_CSV,
        FILE_TYPE_MBC,
        FILE_TYPE_MBS,
        FILE_TYPE_LOG,
        FILE_TYPE_NONE,
    } FileType;

    explicit FileSelectionHelper(QObject *parent = nullptr);

    static void configureFileDialog(QFileDialog* pDialog, GuiModel* pGuiModel, DialogType dialogType, FileType fileType);

signals:

private:
    static void configureDialogType(QFileDialog* pDialog, DialogType dialogType);
    static void configureFileType(QFileDialog* pDialog, FileType fileType);

};

#endif // FILESELECTIONHELPER_H
