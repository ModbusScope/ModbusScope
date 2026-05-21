#ifndef MARKERINFOITEM_H
#define MARKERINFOITEM_H

#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>

#include "util/graphindex.h"

/* Forward declarations */
class GuiModel;
class GraphDataModel;

class MarkerInfoItem : public QFrame
{
    Q_OBJECT

public:
    MarkerInfoItem(QWidget *parent = nullptr);

    void setModel(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel);

signals:

public slots:
    void updateData();
    void updateGraphList(void);

private slots:
    
    void updateColor(GraphIdx graphIdx);
    void updateLabel(GraphIdx graphIdx);
    void removeFromGraphList(GraphIdx index);
    void graphSelected(qint32 index);


private:

    void updateList();
    void selectGraph(qint32 graphIndex);
    double calculateMarkerExpressionValue(quint32 expressionMask);

    QVBoxLayout * _pLayout;
    QComboBox * _pGraphCombo;
    QLabel * _pGraphDataLabelLeft;
    QLabel * _pGraphDataLabelRight;

    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;
};

#endif // MARKERINFOITEM_H
