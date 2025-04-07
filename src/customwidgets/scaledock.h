#ifndef SCALEDOCK_H
#define SCALEDOCK_H

#include <QWidget>
#include <QButtonGroup>

// Forward declaration
class GuiModel;

namespace Ui {
class ScaleDock;
}

class ScaleDock : public QWidget
{
    Q_OBJECT

    friend class TestScaleDock;

public:
    explicit ScaleDock(QWidget *parent = nullptr);
    ~ScaleDock();

    void setModels(GuiModel * pGuiModel);

private slots:
    void handleYMinChange();
    void handleY2MinChange();
    void handleYMaxChange();
    void handleY2MaxChange();

    void updatexAxisSlidingMode();
    void updatexAxisSlidingInterval();
    void updateyAxisSlidingMode();
    void updatey2AxisSlidingMode();
    void updateyAxisMinMax();
    void updatey2AxisMinMax();

    void xAxisScaleGroupClicked(int id);
    void yAxisScaleGroupClicked(int id);
    void y2AxisScaleGroupClicked(int id);

    void selectPrimaryValueAxisTab();
    void selectSecondairyValueAxisTab();

private:

    Ui::ScaleDock *_pUi;
    GuiModel * _pGuiModel;

    QButtonGroup * _pXAxisScaleGroup;
    QButtonGroup * _pYAxisScaleGroup;
    QButtonGroup * _pY2AxisScaleGroup;

};

#endif // SCALEDOCK_H
