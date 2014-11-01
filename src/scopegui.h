#ifndef SCOPEGUI_H
#define SCOPEGUI_H

#include <QObject>
#include <QVector>

// Foward declaration
class QCustomPlot;

class ScopeGui : public QObject
{
    Q_OBJECT
public:
    explicit ScopeGui(QCustomPlot * pPlot, QObject *parent);

    void resetGraph(void);
    void addGraph(quint16 registerAddress);

signals:

public slots:
    void plotResults(bool bSuccess, QList<quint16> values);
    void setYAxisAutoScale(int state);
    void setXAxisAutoScale(int state);
    void exportDataCsv(QString dataFile);
    void exportGraphImage(QString imageFile);

private slots:
    void generateTickLabels();
    void selectionChanged();
    void mousePress();
    void mouseWheel();

private:

    void writeToFile(QString filePath, QString logData);

    typedef struct
    {
        bool bXAxisAutoScale;
        bool bYAxisAutoScale;
    } GuiSettings;

    QCustomPlot * _pPlot;
    qint64 _startTime;

    static const QList<QColor> _colorlist;

    QVector<QString> tickLabels;

    GuiSettings _settings;

    static const quint32 _cMinuteTripPoint = 5*60*1000; /* in ms */
    static const quint32 _cHourTripPoint = 10*60*60*1000; /* in ms */

};

#endif // SCOPEGUI_H
