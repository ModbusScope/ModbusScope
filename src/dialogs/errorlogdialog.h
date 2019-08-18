#ifndef ERRORLOGDIALOG_H
#define ERRORLOGDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QItemSelection>

namespace Ui {
class ErrorLogDialog;
}

// Forward declaration
class ErrorLogModel;
class ErrorLogFilter;

class ErrorLogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorLogDialog(ErrorLogModel * pErrorLogModel, QWidget *parent = nullptr);
    ~ErrorLogDialog();

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

    Ui::ErrorLogDialog *_pUi;
    bool _bAutoScroll;

    ErrorLogModel * _pErrorLogModel;
    ErrorLogFilter* _pCategoryProxyFilter;
    QButtonGroup _categoryFilterGroup;
};

#endif // ERRORLOGDIALOG_H
