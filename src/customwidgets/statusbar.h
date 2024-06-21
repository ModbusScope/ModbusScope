#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QStatusBar>
#include <QLabel>

class GuiModel;

class StatusBar : public QStatusBar
{
    Q_OBJECT
public:
    explicit StatusBar(GuiModel* pGuiModel, QWidget *parent = nullptr);
    ~StatusBar();

    void updateRuntime();

private slots:
    void updateGuiState();
    void updateStats();

private:

    GuiModel* _pGuiModel;

    QLabel * _pStatusStats;
    QLabel * _pStatusState;
    QLabel * _pStatusRuntime;

    static const QString _cStateRunning;
    static const QString _cStateStopped;
    static const QString _cStatsTemplate;
    static const QString _cStateDataLoaded;
    static const QString _cRuntime;
};

#endif // STATUSBAR_H
