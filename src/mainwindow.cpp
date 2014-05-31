#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "modbusscope.h"
#include "QDebug"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _scope(NULL)
{
    ui->setupUi(this);

    _modbusSettings.SetIpAddress("127.0.0.1");
    _modbusSettings.SetPort(1502);

    _scope = new ModbusScope();

    qRegisterMetaType<ModbusSettings>("ModbusSettings");

    connect(ui->btnStartModbus, SIGNAL(released()), this, SLOT(startScope()));
    connect(ui->btnStopModbus, SIGNAL(released()), this, SLOT(stopScope()));

    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);

    ui->customPlot->xAxis->setRange(0, 10);
    ui->customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->customPlot->xAxis->setDateTimeFormat("hh:mm:ss:zzz");
    ui->customPlot->xAxis->setLabel("x Axis");

    ui->customPlot->yAxis->setRange(0, 10);
    ui->customPlot->yAxis->setLabel("y Axis");

    ui->customPlot->plotLayout()->insertRow(0);
    ui->customPlot->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->customPlot, "Interaction Example"));

    ui->customPlot->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->customPlot->legend->setFont(legendFont);

    // connect slot that ties some axis selections together (especially opposite axes):
    connect(ui->customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

}

MainWindow::~MainWindow()
{
    delete _scope;
    delete ui;
}

void MainWindow::startScope(void)
{
    SettingsDialog dialog;
    dialog.setModal(true);
    dialog.exec();
    if (dialog.result() == QDialog::Accepted)
    {
        dialog.getModbusSettings(&_modbusSettings);
        dialog.getRegisterList(&_registerList);

        ui->customPlot->clearGraphs();

        // Add graph(s)
        for (qint32 i = 0; i < _registerList.size(); i++)
        {
            ui->customPlot->addGraph();
        }

        _scope->StartCommunication(&_modbusSettings, &_registerList);
    }
}

void MainWindow::stopScope(void)
{
    _scope->StopCommunication();
}

void MainWindow::plotResults(QList<u_int16_t> values)
{
    double now = QDateTime::currentDateTime().toTime_t();

    qDebug() << "Number of regs to graph (" << values.size() << ")" << "\n";

    for (int32_t i = 0; i < values.size(); i++)
    {
        ui->customPlot->graph(i)->addData(now, (double)values[i]);
    }

    ui->customPlot->rescaleAxes();
    ui->customPlot->replot();
}


void MainWindow::selectionChanged()
{
    /*
    normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
    the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
    and the axis base line together. However, the axis label shall be selectable individually.
    */

    // handle axis and tick labels as one selectable object:
    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) || ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxisLabel))
    {
        ui->customPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spAxisLabel|QCPAxis::spTickLabels);
    }
    // handle axis and tick labels as one selectable object:
    if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) || ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxisLabel))
    {
        ui->customPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spAxisLabel|QCPAxis::spTickLabels);
    }

}

void MainWindow::mousePress()
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged

    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->xAxis->orientation());
    else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->yAxis->orientation());
    else
        ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::mouseWheel()
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->xAxis->orientation());
    else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->yAxis->orientation());
    else
        ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}
