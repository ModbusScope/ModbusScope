#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

#include "markerinfodialog.h"
#include "ui_markerinfodialog.h"


MarkerInfoDialog::MarkerInfoDialog(GuiModel *pGuiModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::MarkerInfoDialog)
{
    _pUi->setupUi(this);

    _pGuiModel = pGuiModel;

    _expressionMask = 0;

    /* Enable/disable custom script controls */
    connect(_pUi->checkCustom, &QCheckBox::stateChanged, this, &MarkerInfoDialog::customScriptStatechanged);

    /* Update mask */
    connect(_pUi->checkAverage, &QCheckBox::stateChanged, this, &MarkerInfoDialog::checkBoxStatechanged);
    connect(_pUi->checkSlope, &QCheckBox::stateChanged, this, &MarkerInfoDialog::checkBoxStatechanged);
    connect(_pUi->checkMaximum, &QCheckBox::stateChanged, this, &MarkerInfoDialog::checkBoxStatechanged);
    connect(_pUi->checkMinimum, &QCheckBox::stateChanged, this, &MarkerInfoDialog::checkBoxStatechanged);
    connect(_pUi->checkDifference, &QCheckBox::stateChanged, this, &MarkerInfoDialog::checkBoxStatechanged);
    connect(_pUi->checkCustom, &QCheckBox::stateChanged, this, &MarkerInfoDialog::checkBoxStatechanged);

    /* Connect file chooser signal */
    connect(_pUi->btnCustom, &QToolButton::clicked, this, &MarkerInfoDialog::selectScriptFile);

    /* set according to data in model */
    _pUi->lineCustom->setText(_pGuiModel->markerExpressionCustomScript());

    const quint32 mask = _pGuiModel->markerExpressionMask();

    if (mask & GuiModel::cAverageMask)
    {
        _pUi->checkAverage->setChecked(true);
    }

    if (mask & GuiModel::cCustomMask)
    {
        _pUi->checkCustom->setChecked(true);
    }

    if (mask & GuiModel::cDifferenceMask)
    {
        _pUi->checkDifference->setChecked(true);
    }

    if (mask & GuiModel::cMaximumMask)
    {
        _pUi->checkMaximum->setChecked(true);
    }

    if (mask & GuiModel::cMinimumMask)
    {
        _pUi->checkMinimum->setChecked(true);
    }

    if (mask & GuiModel::cSlopeMask)
    {
        _pUi->checkSlope->setChecked(true);
    }

}

MarkerInfoDialog::~MarkerInfoDialog()
{
    delete _pUi;
}

void MarkerInfoDialog::customScriptStatechanged(int state)
{
    if (state == Qt::Unchecked)
    {
        _pUi->btnCustom->setEnabled(false);
        _pUi->lineCustom->setEnabled(false);
    }
    else if (state == Qt::Checked)
    {
        _pUi->btnCustom->setEnabled(true);
        _pUi->lineCustom->setEnabled(true);
    }
    else
    {
        /* Nothing to do */
    }
}

void MarkerInfoDialog::checkBoxStatechanged(int state)
{
    QObject* pObj = sender();

    quint32 mask = 0;

    if (pObj == _pUi->checkDifference)
    {
        mask = GuiModel::cDifferenceMask;
    }
    else if (pObj == _pUi->checkSlope)
    {
        mask = GuiModel::cSlopeMask;
    }
    else if (pObj == _pUi->checkAverage)
    {
         mask = GuiModel::cAverageMask;
    }
    else if (pObj == _pUi->checkMinimum)
    {
         mask = GuiModel::cMinimumMask;
    }
    else if (pObj == _pUi->checkMaximum)
    {
         mask = GuiModel::cMaximumMask;
    }
    else if (pObj == _pUi->checkCustom)
    {
        mask = GuiModel::cCustomMask;
    }
    else
    {
        mask = 0u;
    }


    if (state == Qt::Unchecked)
    {
        _expressionMask &= ~(mask);
    }
    else if (state == Qt::Checked)
    {
        _expressionMask |= mask;
    }
    else
    {
        /* Nothing to do */
    }
}

void MarkerInfoDialog::selectScriptFile()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setWindowTitle(tr("Select py script file"));
    dialog.setNameFilter(tr("py files (*.py *.pyc)"));

    if (dialog.exec())
    {
        _pUi->lineCustom->setText(dialog.selectedFiles().first());
    }
}

void MarkerInfoDialog::done(int r)
{
    bool bValid = true;

    if(QDialog::Accepted == r)  // ok was pressed
    {
        if (_pUi->checkCustom->checkState() == Qt::Checked)
        {

            QFileInfo fileInfo = QFileInfo(_pUi->lineCustom->text());

            if (fileInfo.exists())
            {
                bValid = true;
            }
            else
            {
                bValid = false;

                /* TODO: change with Util::ShowError */
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("SCript file does not exist!"));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Script file does not exist.\n\n%1").arg(_pUi->lineCustom->text()));
                msgBox.exec();
            }
        }

        if (bValid)
        {
            _pGuiModel->setMarkerExpressionMask(_expressionMask);
            _pGuiModel->setMarkerExpressionCustomScript(_pUi->lineCustom->text());
        }
    }
    else
    {
        // cancel, close or exc was pressed
        bValid = true;
    }

    if (bValid)
    {
        QDialog::done(r);
    }
}
