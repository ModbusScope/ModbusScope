#ifndef ERRORLOGDIALOG_H
#define ERRORLOGDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QItemSelection>

namespace Ui {
class DiagnosticDialog;
}

// Forward declaration
class DiagnosticFilter;

class DiagnosticDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DiagnosticDialog(QWidget *parent = nullptr);
    ~DiagnosticDialog();

private slots:
    void handleErrorSelectionChanged(QItemSelection selected, QItemSelection deselected);
    void handleLogsChanged();
    void handleCheckAutoScrollChanged(int newState);
    void handleScrollbarChange();
    void handleClearButton();
    void handleFilterChange(int id);

private:
    void setAutoScroll(bool bAutoScroll);
    void updateScroll();
    void updateLogCount();

    Ui::DiagnosticDialog *_pUi;
    bool _bAutoScroll;

    DiagnosticFilter* _pSeverityProxyFilter;
    QButtonGroup _categoryFilterGroup;
};

#endif // ERRORLOGDIALOG_H
