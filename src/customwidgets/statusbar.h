#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QLabel>
#include <QStatusBar>

class GuiModel;
class GraphDataModel;
class CommunicationStatsModel;
class ClickableLabel;

class StatusBar : public QStatusBar
{
    Q_OBJECT
public:
    explicit StatusBar(GuiModel* pGuiModel,
                       GraphDataModel* pGraphDataModel,
                       CommunicationStatsModel* pCommunicationStatsModel,
                       QWidget* parent = nullptr);

private slots:
    void statsClicked();
    void updateGuiState();
    void updateStats();
    void updateTimeStats();

signals:
    void openDiagnostics();

private:
    GuiModel* _pGuiModel;
    GraphDataModel* _pGraphDataModel;
    CommunicationStatsModel* _pCommunicationStatsModel;

    ClickableLabel* _pStatusStats;
    QLabel* _pStatusState;
    QLabel* _pStatusRuntime;

    static const QString _cStateRunning;
    static const QString _cStateStopped;
    static const QString _cStatsTemplate;
    static const QString _cStateDataLoaded;
    static const QString _cRuntime;
    static const QString _cRuntimeWithPoll;
};

#endif // STATUSBAR_H
