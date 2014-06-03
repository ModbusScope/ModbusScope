#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "modbusscope.h"
#include "QDebug"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _scope(NULL)
{
    _ui->setupUi(this);

    _modbusSettings.SetIpAddress("127.0.0.1");
    _modbusSettings.SetPort(1502);

    _scope = new ModbusScope();

    qRegisterMetaType<ModbusSettings>("ModbusSettings");

    connect(_ui->btnStartModbus, SIGNAL(released()), this, SLOT(startScope()));
    connect(_ui->btnStopModbus, SIGNAL(released()), this, SLOT(stopScope()));

    _ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);

    _ui->customPlot->xAxis->setRange(0, 10);
    _ui->customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    _ui->customPlot->xAxis->setDateTimeFormat("hh:mm:ss:zzz");
    _ui->customPlot->xAxis->setLabel("x Axis");

    _ui->customPlot->yAxis->setRange(0, 10);
    _ui->customPlot->yAxis->setLabel("y Axis");

    _ui->customPlot->plotLayout()->insertRow(0);
    _ui->customPlot->plotLayout()->addElement(0, 0, new QCPPlotTitle(_ui->customPlot, "Interaction Example"));

    _ui->customPlot->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    _ui->customPlot->legend->setFont(legendFont);

    // connect slot that ties some axis selections together (especially opposite axes):
    connect(_ui->customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(SelectionChanged()));
    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(_ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(MousePress()));
    connect(_ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(MouseWheel()));

}

MainWindow::~MainWindow()
{
    delete _scope;
    delete _ui;
}

void MainWindow::StartScope()
{
    SettingsDialog dialog;
    dialog.setModal(true);
    dialog.exec();
    if (dialog.result() == QDialog::Accepted)
    {
        dialog.GetModbusSettings(&_modbusSettings);
        dialog.GetRegisterList(&_registerList);

        _ui->customPlot->clearGraphs();

        // Add graph(s)
        for (qint32 i = 0; i < _registerList.size(); i++)
        {
            _ui->customPlot->addGraph();
        }

        _scope->StartCommunication(&_modbusSettings, &_registerList);
    }
}

void MainWindow::StopScope()
{
    _scope->StopCommunication();
}

void MainWindow::PlotResults(QList<quint16> values)
{
    double now = QDateTime::currentDateTime().toTime_t();

    qDebug() << "Number of regs to graph (" << values.size() << ")" << "\n";

    for (qint32 i = 0; i < values.size(); i++)
    {
        _ui->customPlot->graph(i)->addData(now, (double)values[i]);
    }

    _ui->customPlot->rescaleAxes();
    _ui->customPlot->replot();
}


void MainWindow::SelectionChanged()
{
    /*
    normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
    the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
    and the axis base line together. However, the axis label shall be selectable individually.
    */

    // handle axis and tick labels as one selectable object:
    if (_ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || _ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) || _ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxisLabel))
    {
        _ui->customPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spAxisLabel|QCPAxis::spTickLabels);
    }
    // handle axis and tick labels as one selectable object:
    if (_ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || _ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) || _ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxisLabel))
    {
        _ui->customPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spAxisLabel|QCPAxis::spTickLabels);
    }

}

void MainWindow::MousePress()
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged

    if (_ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        _ui->customPlot->axisRect()->setRangeDrag(_ui->customPlot->xAxis->orientation());
    else if (_ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        _ui->customPlot->axisRect()->setRangeDrag(_ui->customPlot->yAxis->orientation());
    else
        _ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::MouseWheel()
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (_ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        _ui->customPlot->axisRect()->setRangeZoom(_ui->customPlot->xAxis->orientation());
    else if (_ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        _ui->customPlot->axisRect()->setRangeZoom(_ui->customPlot->yAxis->orientation());
    else
        _ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}
