#ifndef MARKERINFOITEM_H
#define MARKERINFOITEM_H

#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>


/* Forward declarations */
class GuiModel;
class GraphDataModel;

class MarkerInfoItem : public QFrame
{
    Q_OBJECT

public:
    MarkerInfoItem(QWidget *parent = 0);

    void setModel(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel);

signals:

public slots:
    void updateData();
    void updateGraphList(void);

private slots:
    
    void updateColor(quint32 graphIdx);
    void updateLabel(quint32 graphIdx);
    void removeFromGraphList(const quint32 index);
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
