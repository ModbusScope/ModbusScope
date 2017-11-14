#ifndef ERRORLOGDIALOG_H
#define ERRORLOGDIALOG_H

#include <QDialog>
#include <QItemSelection>

namespace Ui {
class ErrorLogDialog;
}

// Forward declaration
class ErrorLogModel;

class ErrorLogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorLogDialog(ErrorLogModel * pErrorLogModel, QWidget *parent = 0);
    ~ErrorLogDialog();

private slots:
    void handleErrorSelectionChanged(QItemSelection selected, QItemSelection deselected);
    void handleLogsInserted();
    void handleCheckAutoScrollChanged(int newState);
    void handleScrollbarChange();
    void handleClearButton();

private:
    void setAutoScroll(bool bAutoScroll);
    void updateScroll();

    Ui::ErrorLogDialog *_pUi;
    bool _bAutoScroll;

    ErrorLogModel * _pErrorLogModel;
};

#endif // ERRORLOGDIALOG_H
