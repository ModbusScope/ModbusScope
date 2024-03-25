#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QWidgetAction>

/* Forward declaration */
class GraphDataModel;
class GuiModel;
class SettingsModel;
class RegisterValueAxisDelegate;
class ExpressionDelegate;
class GraphData;

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(GuiModel * pGuiModel, GraphDataModel *pGraphDataModel, SettingsModel* pSettingsModel, QWidget *parent = nullptr);
    ~RegisterDialog();

public slots:
    int execWithMbcImport();

private slots:
    void showImportDialog();
    void addRegister(const GraphData &graphData);
    void addDefaultRegister();
    void removeRegisterRow();
    void activatedCell(QModelIndex modelIndex);
    void onRegisterInserted(const QModelIndex &parent, int first, int last);
    void handleExpressionEdit(int row);

private:
    int selectedRowAfterDelete(int deletedStartIndex, int rowCnt);
    static bool sortRegistersLastFirst(const QModelIndex &s1, const QModelIndex &s2);

    Ui::RegisterDialog* _pUi;

    GraphDataModel* _pGraphDataModel;
    GuiModel* _pGuiModel;
    SettingsModel* _pSettingsModel;

    std::unique_ptr<RegisterValueAxisDelegate> _valueAxisDelegate;
    std::unique_ptr<ExpressionDelegate> _expressionDelegate;
    std::unique_ptr<QWidgetAction> _registerPopupAction;

};

#endif // REGISTERDIALOG_H
