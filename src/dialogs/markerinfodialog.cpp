#include <QFileInfo>
#include <QFileDialog>

#include "util.h"
#include "markerinfodialog.h"
#include "ui_markerinfodialog.h"


MarkerInfoDialog::MarkerInfoDialog(GuiModel *pGuiModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::MarkerInfoDialog)
{
    _pUi->setupUi(this);

    _pGuiModel = pGuiModel;

    _expressionMask = 0;

    /* Update mask */
    connect(_pUi->checkAverage, &QCheckBox::stateChanged, this, &MarkerInfoDialog::checkBoxStatechanged);
    connect(_pUi->checkSlope, &QCheckBox::stateChanged, this, &MarkerInfoDialog::checkBoxStatechanged);
    connect(_pUi->checkMaximum, &QCheckBox::stateChanged, this, &MarkerInfoDialog::checkBoxStatechanged);
    connect(_pUi->checkMinimum, &QCheckBox::stateChanged, this, &MarkerInfoDialog::checkBoxStatechanged);
    connect(_pUi->checkDifference, &QCheckBox::stateChanged, this, &MarkerInfoDialog::checkBoxStatechanged);

    const quint32 mask = _pGuiModel->markerExpressionMask();

    if (mask & GuiModel::cAverageMask)
    {
        _pUi->checkAverage->setChecked(true);
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

void MarkerInfoDialog::done(int r)
{
    bool bValid = true;

    if(QDialog::Accepted == r)  // ok was pressed
    {
        if (bValid)
        {
            _pGuiModel->setMarkerExpressionMask(_expressionMask);
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
