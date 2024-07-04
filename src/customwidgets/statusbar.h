#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QStatusBar>
#include <QLabel>

class GuiModel;
class ClickableLabel;

class StatusBar : public QStatusBar
{
    Q_OBJECT
public:
    explicit StatusBar(GuiModel* pGuiModel, QWidget *parent = nullptr);

    void updateRuntime();

private slots:
    void statsClicked();
    void updateGuiState();
    void updateStats();

signals:
    void openDiagnostics();

private:

    GuiModel* _pGuiModel;

    ClickableLabel * _pStatusStats;
    QLabel * _pStatusState;
    QLabel * _pStatusRuntime;

    static const QString _cStateRunning;
    static const QString _cStateStopped;
    static const QString _cStatsTemplate;
    static const QString _cStateDataLoaded;
    static const QString _cRuntime;
};

#endif // STATUSBAR_H
