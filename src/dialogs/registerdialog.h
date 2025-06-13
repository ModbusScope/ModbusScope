#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include "customwidgets/centeredbox.h"

#include <QDialog>
#include <QWidgetAction>

/* Forward declaration */
class GraphDataModel;
class SettingsModel;
class RegisterValueAxisDelegate;
class ActionButtonDelegate;
class GraphData;

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(GraphDataModel* pGraphDataModel, SettingsModel* pSettingsModel, QWidget* parent = nullptr);
    ~RegisterDialog();

private slots:
    void addRegister(const GraphData &graphData);
    void addDefaultRegister();
    void removeRegisterRow();
    void activatedCell(QModelIndex modelIndex);
    void onRegisterInserted(const QModelIndex &parent, int first, int last);
    void handleExpressionEdit(const QModelIndex& index);

private:
    int selectedRowAfterDelete(int deletedStartIndex, int rowCnt);
    static bool sortRegistersLastFirst(const QModelIndex &s1, const QModelIndex &s2);

    Ui::RegisterDialog* _pUi;

    GraphDataModel* _pGraphDataModel;
    SettingsModel* _pSettingsModel;

    CenteredBoxProxyStyle _centeredBoxStyle;

    std::unique_ptr<RegisterValueAxisDelegate> _valueAxisDelegate;
    std::unique_ptr<ActionButtonDelegate> _expressionDelegate;
    std::unique_ptr<QWidgetAction> _registerPopupAction;

};

#endif // REGISTERDIALOG_H
