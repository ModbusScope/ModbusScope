#ifndef IMPORTMBCDIALOG_H
#define IMPORTMBCDIALOG_H

#include <QDialog>

#include "customwidgets/centeredbox.h"
#include "models/mbcregistermodel.h"

/* Forward declaration */
class GuiModel;
class GraphDataModel;
class MbcRegisterFilter;
class MbcUpdateModel;

namespace Ui {
class ImportMbcDialog;
}

class ImportMbcDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportMbcDialog(GuiModel* pGuiModel, GraphDataModel* _pGraphDatamodel, QWidget* parent = nullptr);
    ~ImportMbcDialog();

public slots:
    int exec(void);

private slots:
    void updateTextFilter();
    void selectMbcFile();
    void importSelectedRegisters();
    void visibleItemsDataChanged();
    void registerDataChanged();
    void handleSelectAllClicked(Qt::CheckState state);

private:
    void setSelectedSelectionstate(Qt::CheckState state);
    void updateMbcRegisters(QString filePath);

    Ui::ImportMbcDialog *_pUi;

    CenteredBoxProxyStyle _centeredBoxStyle;

    GuiModel * _pGuiModel;
    GraphDataModel* _pGraphDataModel;
    MbcRegisterModel _mbcRegisterModel;
    MbcUpdateModel* _pMbcUpdateModel;

    MbcRegisterFilter * _pTabProxyFilter;
};

#endif // IMPORTMBCDIALOG_H
