
#include "fileselectionhelper.h"

#include "models/guimodel.h"

#include <QFileDialog>

GuiModel* FileSelectionHelper::_pGuiModel;

FileSelectionHelper::FileSelectionHelper(QObject *parent) : QObject(parent)
{

}

void FileSelectionHelper::configureFileDialog(QFileDialog* pDialog, DialogType dialogType, FileType fileType)
{
    configureDialogType(pDialog, dialogType);
    configureFileType(pDialog, fileType);

    if (_pGuiModel)
    {
        pDialog->setDirectory(_pGuiModel->lastDir());
    }
}

void FileSelectionHelper::setGuiModel(GuiModel* pGuiModel)
{
    _pGuiModel = pGuiModel;
}

QString FileSelectionHelper::showDialog(QFileDialog* pDialog)
{
    QString selectedFile;

    if (pDialog->exec() == QDialog::Accepted)
    {
        auto fileList = pDialog->selectedFiles();
        if (!fileList.isEmpty())
        {
            selectedFile = fileList.at(0);

            _pGuiModel->setLastDir(QFileInfo(selectedFile).dir().absolutePath());
        }
    }

    return selectedFile;
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
