#include "scaledock.h"

#include "models/guimodel.h"
#include "ui_scaledock.h"
#include "util/util.h"

ScaleDock::ScaleDock(QWidget *parent) :
    QWidget(parent),
    _pUi(new Ui::ScaleDock),
    _pGuiModel(nullptr)
{
    _pUi->setupUi(this);

    QDoubleValidator* pValidator = new QDoubleValidator(-1000000, 1000000, 3); /* Util:formatdouble is also 3 decimals */
    _pUi->lineYMin->setValidator(pValidator);
    _pUi->lineY2Min->setValidator(pValidator);
    _pUi->lineYMax->setValidator(pValidator);
    _pUi->lineY2Max->setValidator(pValidator);
    connect(_pUi->lineYMin, &QLineEdit::editingFinished, this, &ScaleDock::handleYMinChange);
    connect(_pUi->lineY2Min, &QLineEdit::editingFinished, this, &ScaleDock::handleY2MinChange);
    connect(_pUi->lineYMax, &QLineEdit::editingFinished, this, &ScaleDock::handleYMaxChange);
    connect(_pUi->lineY2Max, &QLineEdit::editingFinished, this, &ScaleDock::handleY2MaxChange);

    //valueChanged is only send when done editing...
    _pUi->spinSlidingXInterval->setKeyboardTracking(false);

    // Create button group for X axis scaling options
    _pXAxisScaleGroup = new QButtonGroup();
    _pXAxisScaleGroup->setExclusive(true);
    _pXAxisScaleGroup->addButton(_pUi->radioXFullScale, AxisMode::SCALE_AUTO);
    _pXAxisScaleGroup->addButton(_pUi->radioXSliding, AxisMode::SCALE_SLIDING);
    _pXAxisScaleGroup->addButton(_pUi->radioXManual, AxisMode::SCALE_MANUAL);
    connect(_pXAxisScaleGroup, &QButtonGroup::idClicked, this, &ScaleDock::xAxisScaleGroupClicked);

    _pUi->scaleTab->setCurrentIndex(0);

    // Create button group for Y axis scaling options
    _pYAxisScaleGroup = new QButtonGroup();
    _pYAxisScaleGroup->setExclusive(true);
    _pYAxisScaleGroup->addButton(_pUi->radioYFullScale, AxisMode::SCALE_AUTO);
    _pYAxisScaleGroup->addButton(_pUi->radioYWindowScale, AxisMode::SCALE_WINDOW_AUTO);
    _pYAxisScaleGroup->addButton(_pUi->radioYMinMax, AxisMode::SCALE_MINMAX);
    _pYAxisScaleGroup->addButton(_pUi->radioYManual, AxisMode::SCALE_MANUAL);
    connect(_pYAxisScaleGroup, &QButtonGroup::idClicked, this, &ScaleDock::yAxisScaleGroupClicked);

    // Create button group for Y2 axis scaling options
    _pY2AxisScaleGroup = new QButtonGroup();
    _pY2AxisScaleGroup->setExclusive(true);
    _pY2AxisScaleGroup->addButton(_pUi->radioY2FullScale, AxisMode::SCALE_AUTO);
    _pY2AxisScaleGroup->addButton(_pUi->radioY2WindowScale, AxisMode::SCALE_WINDOW_AUTO);
    _pY2AxisScaleGroup->addButton(_pUi->radioY2MinMax, AxisMode::SCALE_MINMAX);
    _pY2AxisScaleGroup->addButton(_pUi->radioY2Manual, AxisMode::SCALE_MANUAL);
    connect(_pY2AxisScaleGroup, &QButtonGroup::idClicked, this, &ScaleDock::y2AxisScaleGroupClicked);
}

ScaleDock::~ScaleDock()
{
    delete _pUi;
}

void ScaleDock::setModels(GuiModel * pGuiModel)
{
    _pGuiModel = pGuiModel;

    connect(_pUi->spinSlidingXInterval, QOverload<int>::of(&QSpinBox::valueChanged), _pGuiModel, &GuiModel::setxAxisSlidingInterval);

    connect(_pGuiModel, &GuiModel::xAxisScalingChanged, this, &ScaleDock::updatexAxisSlidingMode);
    connect(_pGuiModel, &GuiModel::xAxisSlidingIntervalChanged, this, &ScaleDock::updatexAxisSlidingInterval);
    connect(_pGuiModel, &GuiModel::yAxisScalingChanged, this, &ScaleDock::updateyAxisSlidingMode);
    connect(_pGuiModel, &GuiModel::y2AxisScalingChanged, this, &ScaleDock::updatey2AxisSlidingMode);
    connect(_pGuiModel, &GuiModel::yAxisMinMaxchanged, this, &ScaleDock::updateyAxisMinMax);
    connect(_pGuiModel, &GuiModel::y2AxisMinMaxchanged, this, &ScaleDock::updatey2AxisMinMax);

    connect(_pGuiModel, &GuiModel::yAxisSelected, this, &ScaleDock::selectPrimaryValueAxisTab);
    connect(_pGuiModel, &GuiModel::y2AxisSelected, this, &ScaleDock::selectSecondairyValueAxisTab);
}

void ScaleDock::handleYMinChange()
{
    bool bOk = false;
    double val = QLocale().toDouble(_pUi->lineYMin->text(), &bOk);

    if (bOk)
    {
        _pGuiModel->setyAxisMin(val);
        _pGuiModel->setyAxisScale(AxisMode::SCALE_MINMAX);
    }
}

void ScaleDock::handleY2MinChange()
{
    bool bOk = false;
    double val = QLocale().toDouble(_pUi->lineY2Min->text(), &bOk);

    if (bOk)
    {
        _pGuiModel->sety2AxisMin(val);
        _pGuiModel->sety2AxisScale(AxisMode::SCALE_MINMAX);
    }
}

void ScaleDock::handleYMaxChange()
{
    bool bOk = false;
    double val = QLocale().toDouble(_pUi->lineYMax->text(), &bOk);

    if (bOk)
    {
        _pGuiModel->setyAxisMax(val);
        _pGuiModel->setyAxisScale(AxisMode::SCALE_MINMAX);
    }
}

void ScaleDock::handleY2MaxChange()
{
    bool bOk = false;
    double val = QLocale().toDouble(_pUi->lineY2Max->text(), &bOk);

    if (bOk)
    {
        _pGuiModel->sety2AxisMax(val);
        _pGuiModel->sety2AxisScale(AxisMode::SCALE_MINMAX);
    }
}

void ScaleDock::updatexAxisSlidingMode()
{
    if (_pGuiModel->xAxisScalingMode() == AxisMode::SCALE_AUTO)
    {
        // Full auto scaling
        _pUi->radioXFullScale->setChecked(true);
    }
    else if (_pGuiModel->xAxisScalingMode() == AxisMode::SCALE_SLIDING)
    {
        // Sliding window
        _pUi->radioXSliding->setChecked(true);
    }
    else
    {
        // manual
        _pUi->radioXManual->setChecked(true);
    }
}

void ScaleDock::updatexAxisSlidingInterval()
{
    _pUi->spinSlidingXInterval->setValue(_pGuiModel->xAxisSlidingSec());
}

void ScaleDock::updateyAxisSlidingMode()
{
    if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_AUTO)
    {
        // Full auto scaling
        _pUi->radioYFullScale->setChecked(true);
    }
    else if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_MINMAX)
    {
        // Min and max selected
        _pUi->radioYMinMax->setChecked(true);
    }
    else if (_pGuiModel->yAxisScalingMode() == AxisMode::SCALE_WINDOW_AUTO)
    {
        // Window auto scale selected
        _pUi->radioYWindowScale->setChecked(true);
    }
    else
    {
        // manual
        _pUi->radioYManual->setChecked(true);
    }
}

void ScaleDock::updatey2AxisSlidingMode()
{
    if (_pGuiModel->y2AxisScalingMode() == AxisMode::SCALE_AUTO)
    {
        // Full auto scaling
        _pUi->radioY2FullScale->setChecked(true);
    }
    else if (_pGuiModel->y2AxisScalingMode() == AxisMode::SCALE_MINMAX)
    {
        // Min and max selected
        _pUi->radioY2MinMax->setChecked(true);
    }
    else if (_pGuiModel->y2AxisScalingMode() == AxisMode::SCALE_WINDOW_AUTO)
    {
        // Window auto scale selected
        _pUi->radioY2WindowScale->setChecked(true);
    }
    else
    {
        // manual
        _pUi->radioY2Manual->setChecked(true);
    }
}

void ScaleDock::updateyAxisMinMax()
{
    _pUi->lineYMin->setText(Util::formatDoubleForExport(_pGuiModel->yAxisMin()));
    _pUi->lineYMax->setText(Util::formatDoubleForExport(_pGuiModel->yAxisMax()));
}

void ScaleDock::updatey2AxisMinMax()
{
    _pUi->lineY2Min->setText(Util::formatDoubleForExport(_pGuiModel->y2AxisMin()));
    _pUi->lineY2Max->setText(Util::formatDoubleForExport(_pGuiModel->y2AxisMax()));
}

void ScaleDock::xAxisScaleGroupClicked(int id)
{
    _pGuiModel->setxAxisScale((AxisMode::AxisScaleOptions)id);
}

void ScaleDock::yAxisScaleGroupClicked(int id)
{
    _pGuiModel->setyAxisScale((AxisMode::AxisScaleOptions)id) ;
}

void ScaleDock::y2AxisScaleGroupClicked(int id)
{
    _pGuiModel->sety2AxisScale((AxisMode::AxisScaleOptions)id) ;
}

void ScaleDock::selectPrimaryValueAxisTab()
{
    _pUi->scaleTab->setCurrentIndex(0);
}

void ScaleDock::selectSecondairyValueAxisTab()
{
    _pUi->scaleTab->setCurrentIndex(1);
}
