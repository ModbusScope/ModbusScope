#ifndef MARKERINFO_H
#define MARKERINFO_H

#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>


/* Forward declarations */
class GuiModel;
class GraphDataModel;
class MarkerInfoItem;

class MarkerInfo : public QFrame
{
    Q_OBJECT

public:
    MarkerInfo(QWidget *parent = 0);

    void setModel(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel);

signals:

public slots:

private slots:
	void updateMarkerData();

private:

    QVBoxLayout * _pLayout;
    QLabel * _pTimeDataLabel;

    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;
    
    QList<MarkerInfoItem *> graphMarkerData;
    
    static const quint32 graphMarkerCount = 3;
};

#endif // MARKERINFO_H
