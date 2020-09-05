#include "expressionsdialog.h"
#include "ui_expressionsdialog.h"

#include "graphdatamodel.h"

ExpressionsDialog::ExpressionsDialog(GraphDataModel *pGraphDataModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ExpressionsDialog),
    _pGraphDataModel(pGraphDataModel)
{
    _pUi->setupUi(this);

    /* Fill combo box */

    _pUi->widgetInfo->setVisible(false);
    connect(_pUi->btnInfo, &QAbstractButton::toggled, _pUi->widgetInfo, &QWidget::setVisible);
}

ExpressionsDialog::~ExpressionsDialog()
{
    delete _pUi;
}
