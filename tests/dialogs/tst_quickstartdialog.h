#ifndef TST_QUICKSTARTDIALOG_H
#define TST_QUICKSTARTDIALOG_H

#include <QObject>

class TestQuickStartDialog : public QObject
{
    Q_OBJECT

private slots:
    void dontShowAgainDefaultsToFalse();
    void setDontShowAgainRoundTrip();
    void showDocsButtonEmitsSignal();
    void loadDemoButtonEmitsSignal();
    void closeButtonRejects();
};

#endif // TST_QUICKSTARTDIALOG_H
