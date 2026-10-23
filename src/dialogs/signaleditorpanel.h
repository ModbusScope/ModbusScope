#ifndef SIGNALEDITORPANEL_H
#define SIGNALEDITORPANEL_H

#include <QButtonGroup>
#include <QWidget>

class DataPointEditorWidget;
class DataPointService;
class GraphDataModel;
class SettingsModel;

namespace Ui {
class SignalEditorPanel;
}

class SignalEditorPanel : public QWidget
{
    Q_OBJECT

    friend class TestSignalEditorPanel;

public:
    explicit SignalEditorPanel(GraphDataModel* pGraphDataModel,
                               SettingsModel* pSettingsModel,
                               DataPointService* pDataPointService,
                               QWidget* parent = nullptr);
    ~SignalEditorPanel();

    void startAdd();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void handleAdd();
    void onExpressionChanged();
    void setBuildError(const QString& error);
    void updateStatus();

private:
    Ui::SignalEditorPanel* _pUi;
    DataPointEditorWidget* _pEditor;
    GraphDataModel* _pGraphDataModel;

    QButtonGroup _axisGroup;

    QString _buildError;
    QString _addedMessage;
};

#endif // SIGNALEDITORPANEL_H
