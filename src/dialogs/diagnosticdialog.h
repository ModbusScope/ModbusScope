#ifndef DIAGNOSTICDIALOG_H
#define DIAGNOSTICDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QItemSelection>

namespace Ui {
class DiagnosticDialog;
}

// Forward declaration
class DiagnosticFilter;
class DiagnosticModel;

class DiagnosticDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DiagnosticDialog(DiagnosticModel *pDiagnosticModel, QWidget *parent = nullptr);
    ~DiagnosticDialog();

private slots:
    void handleErrorSelectionChanged(QItemSelection selected, QItemSelection deselected);
    void handleLogsChanged();
    void handleCheckAutoScrollChanged(int newState);
    void handleScrollbarChange();
    void handleClearButton();
    void handleFilterChange(int id);
    void handleEnableDebugLog(int state);

private:
    void setAutoScroll(bool bAutoScroll);
    void updateScroll();
    void updateLogCount();

    Ui::DiagnosticDialog *_pUi;
    bool _bAutoScroll;

    DiagnosticModel * _pDiagnosticModel;
    DiagnosticFilter* _pSeverityProxyFilter;
    QButtonGroup _categoryFilterGroup;
};

#endif // DIAGNOSTICDIALOG_H
