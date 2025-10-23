#ifndef DIAGNOSTICDIALOG_H
#define DIAGNOSTICDIALOG_H

#include <QDialog>
#include <QMenu>
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
    explicit DiagnosticDialog(DiagnosticModel* pDiagnosticModel, QWidget* parent = nullptr);
    ~DiagnosticDialog();

private slots:
    void handleErrorSelectionChanged(QItemSelection selected, QItemSelection deselected);
    void handleLogsChanged();
    void handleCheckAutoScrollChanged(Qt::CheckState state);
    void handleScrollbarChange();
    void handleClearButton();
    void handleFilterChange();
    void handleEnableDebugLog(Qt::CheckState state);
    void handleExportLog();
    void showContextMenu(const QPoint& pos);
    void handleCopyDiagnostics();

private:
    void setAutoScroll(bool bAutoScroll);
    void updateScroll();
    void updateLogCount();

    Ui::DiagnosticDialog*_pUi;
    bool _bAutoScroll;

    DiagnosticModel* _pDiagnosticModel;
    DiagnosticFilter* _pSeverityProxyFilter;

    QButtonGroup _categoryFilterGroup;

    QMenu * _pDiagnosticMenu;
    QAction * _pCopyDiagnosticAction;
};

#endif // DIAGNOSTICDIALOG_H
