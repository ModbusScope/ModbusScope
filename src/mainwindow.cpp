#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    worker(NULL)
{
    ui->setupUi(this);

    if (!worker)
    {
        worker = new ModbusThread();

        connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
        connect(worker, SIGNAL(done()), this, SLOT(workerStopped()));
        connect(worker, SIGNAL(done()), worker, SLOT(deleteLater()));
        connect(worker, SIGNAL(modbusResults(unsigned short,unsigned short)), this, SLOT(plotResults(unsigned short,unsigned short)));

        connect(this, SIGNAL(closing()), worker, SLOT(stopThread()));

        connect(ui->btnStartModbus, SIGNAL(released()), worker, SLOT(startCommunication(/*QList<unsigned short>*/)));
        connect(ui->btnStopModbus, SIGNAL(released()), worker, SLOT(stopCommunication()));

        worker->start();
    }

    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
    ui->customPlot->xAxis->setRange(0, 10);
    ui->customPlot->yAxis->setRange(0, 10);

    ui->customPlot->plotLayout()->insertRow(0);
    ui->customPlot->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->customPlot, "Interaction Example"));

    // Add 2 graphs
    ui->customPlot->addGraph();
    ui->customPlot->addGraph();

    ui->customPlot->xAxis->setLabel("x Axis");
    ui->customPlot->yAxis->setLabel("y Axis");
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
    delete ui;
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    event = event;

    emit closing();

    if (worker)
    {
        worker->wait();
    }
}

void MainWindow::errorString(QString error)
{
    qDebug() << "Error: " << error << "\n";
}


void MainWindow::workerStopped()
{
    worker = NULL;
}

void MainWindow::plotResults(unsigned short result0, unsigned short result1)
{
    double now = QDateTime::currentDateTime().toTime_t();

    ui->customPlot->graph(0)->addData(now, (double)result0);
    ui->customPlot->graph(1)->addData(now, (double)result1);

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
