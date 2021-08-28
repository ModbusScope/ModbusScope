
#include "fileselectionhelper.h"
#include "guimodel.h"
#include <QFileDialog>

FileSelectionHelper::FileSelectionHelper(QObject *parent) : QObject(parent)
{

}

void FileSelectionHelper::configureFileDialog(QFileDialog* pDialog, GuiModel* pGuiModel, DialogType dialogType, FileType fileType)
{
    configureDialogType(pDialog, dialogType);
    configureFileType(pDialog, fileType);

    pDialog->setDirectory(pGuiModel->lastDir());
}

void FileSelectionHelper::configureDialogType(QFileDialog* pDialog, DialogType dialogType)
{
    switch (dialogType)
    {
    case DIALOG_TYPE_OPEN:
        pDialog->setFileMode(QFileDialog::ExistingFile);
        pDialog->setAcceptMode(QFileDialog::AcceptOpen);
        break;

    case DIALOG_TYPE_SAVE:
        pDialog->setFileMode(QFileDialog::AnyFile);
        pDialog->setAcceptMode(QFileDialog::AcceptSave);
        break;

    default:
        break;
    }

    pDialog->setOption(QFileDialog::HideNameFilterDetails, false);
}

void FileSelectionHelper::configureFileType(QFileDialog* pDialog, FileType fileType)
{
    switch (fileType)
    {
    case FILE_TYPE_MBS:
        pDialog->setDefaultSuffix("mbs");
        pDialog->setWindowTitle(tr("Select mbs file"));
        pDialog->setNameFilter(tr("MBS files (*.mbs)"));
        break;

    case FILE_TYPE_CSV:
        pDialog->setDefaultSuffix("csv");
        pDialog->setWindowTitle(tr("Select csv file"));
        pDialog->setNameFilter(tr("CSV files (*.csv)"));
        break;

    case FILE_TYPE_MBC:
        pDialog->setDefaultSuffix("mbc");
        pDialog->setWindowTitle(tr("Select mbc file"));
        pDialog->setNameFilter(tr("MBC files (*.mbc)"));
        break;

    case FILE_TYPE_PNG:
        pDialog->setDefaultSuffix("png");
        pDialog->setWindowTitle(tr("Select png file"));
        pDialog->setNameFilter(tr("PNG files (*.png)"));
        break;

    case FILE_TYPE_LOG:
        pDialog->setDefaultSuffix("log");
        pDialog->setWindowTitle(tr("Select log file"));
        pDialog->setNameFilter(tr("LOG files (*.log)"));
        break;

    case FILE_TYPE_NONE:
        break;

    default:
        break;
    }
}
