#ifndef MARKERINFO_H
#define MARKERINFO_H

#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>


/* Forward declarations */
class GuiModel;
class GraphDataModel;

class MarkerInfo : public QFrame
{
    Q_OBJECT

public:
    MarkerInfo(QWidget *parent = 0);

    void setModel(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel);

signals:

public slots:
    void updateMarkerData();

private slots:
    void updateGraphList(void);
    void removeFromGraphList(const quint32 index);
    void graphSelected(qint32 index);

private:

    void updateList();
    void selectGraph(qint32 graphIndex);

    QVBoxLayout * _pLayout;
    QComboBox * _pGraphCombo;
    QLabel * _pTimeDataLabel;
    QLabel * _pGraphDataLabel;

    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;
};

#endif // MARKERINFO_H
