#include "importmbcdialog.h"
#include "ui_importmbcdialog.h"


#include "util.h"
#include "guimodel.h"
#include "mbcfileimporter.h"

#include <QFileDialog>

ImportMbcDialog::ImportMbcDialog(GuiModel * pGuiModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ImportMbcDialog)
{
    _pUi->setupUi(this);

    _pGuiModel = pGuiModel;

    /* Disable question mark button */
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    /*
     * 0: checkbox
     * 1: register address
     * 2: Text
     * 3: Unsigned
     * 4: TabName
     * */
    _pUi->tblMbcRegisters->setColumnCount(5);
    _pUi->tblMbcRegisters->setSortingEnabled(false);

    QStringList headerNames = QStringList() << "" << "Address" << "Text" << "Unsigned" << "Tab";
    _pUi->tblMbcRegisters->setHorizontalHeaderLabels(headerNames);

    connect(_pUi->btnSelectMbcFile, SIGNAL(clicked()), this, SLOT(selectMbcFile()));

    //Add checkbox to row: void QTableWidget::setCellWidget ( int row, int column, QWidget * widget )
}

ImportMbcDialog::~ImportMbcDialog()
{
    delete _pUi;
}


int ImportMbcDialog::exec()
{
    updateMbcRegisters();

    return QDialog::exec();
}

void ImportMbcDialog::selectMbcFile()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setDefaultSuffix("mbc");
    dialog.setWindowTitle(tr("Select mbc file"));
    dialog.setNameFilter(tr("MBC files (*.mbc)"));
    dialog.setDirectory(_pGuiModel->lastDir());

    if (dialog.exec() == QDialog::Accepted)
    {
        _mbcFilePath = dialog.selectedFiles().first();
        _pUi->lineMbcfile->setText(_mbcFilePath);

        _pGuiModel->setLastDir(QFileInfo(_mbcFilePath).dir().absolutePath());

        if (QFile(_mbcFilePath).exists())
        {
            if (updateMbcRegisters())
            {

            }
            else
            {

            }
        }
        else
        {
            Util::showError("No valid MBC file selected.");
        }
    }
}


bool ImportMbcDialog::updateMbcRegisters()
{
    bool bSuccess = false;

    MbcFileImporter fileImporter(_mbcFilePath);
    QList <MbcRegisterData> registerList;

    /* Clear data from table widget */
    _pUi->tblMbcRegisters->clearContents();

    if (fileImporter.parseRegisters(&registerList))
    {
        QList<qint16> regList;

        /* Create rows and columns */
        _pUi->tblMbcRegisters->setRowCount(registerList.count());
        _pUi->tblMbcRegisters->setColumnCount(5);

        // Fill widget with data
        for (qint32 row = 0; row < registerList.size(); row++)
        {
            const MbcRegisterData registerData = registerList[row];

            /* Disable all flags */
            Qt::ItemFlags flags = Qt::NoItemFlags;
            QString toolTipTxt;

            /* Disable all 32 bits registers and duplicates */
            if (regList.contains(registerData.registerAddress))
            {
                toolTipTxt = tr("Duplicate address");
            }
            else if (registerData.bUint32)
            {
                toolTipTxt = tr("32 bit register is not supported");
            }
            else
            {
                flags |= Qt::ItemIsEnabled;
            }


            for(qint32 column = 0; column <  _pUi->tblMbcRegisters->columnCount(); column++)
            {
                /* Create cell */
                QTableWidgetItem *item = new QTableWidgetItem;
                _pUi->tblMbcRegisters->setItem(row, column, item);

                /* Set tooltip if required */
                if (!toolTipTxt.isEmpty())
                {
                    item->setToolTip(toolTipTxt);
                }

                /* Set flags */
                item->setFlags(flags);
            }

            /* Checkbox */
            _pUi->tblMbcRegisters->item(row, 0)->setFlags(flags | Qt::ItemIsUserCheckable);
            _pUi->tblMbcRegisters->item(row, 0)->setCheckState(Qt::Unchecked);

            /* Register address */
            _pUi->tblMbcRegisters->item(row, 1)->setText(QString("%1").arg(registerData.registerAddress));

            /* Text */
            _pUi->tblMbcRegisters->item(row, 2)->setText(registerData.name);

            /* unsigned */
            _pUi->tblMbcRegisters->item(row, 3)->setFlags(flags | Qt::ItemIsUserCheckable);
            if (registerData.bUnsigned)
            {
                _pUi->tblMbcRegisters->item(row, 3)->setCheckState(Qt::Checked);
            }
            else
            {
                _pUi->tblMbcRegisters->item(row, 3)->setCheckState(Qt::Unchecked);
            }

            /* TabName */
            _pUi->tblMbcRegisters->item(row, 4)->setText(registerData.tabName);

        }

        /* Resize */
        _pUi->tblMbcRegisters->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);


        bSuccess = true;
    }


    return bSuccess;
}

