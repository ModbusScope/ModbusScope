#ifndef IMPORTMBCDIALOG_H
#define IMPORTMBCDIALOG_H

#include <QDialog>

#include "customwidgets/centeredbox.h"

/* Forward declaration */
class GuiModel;
class MbcRegisterModel;
class MbcRegisterFilter;

namespace Ui {
class ImportMbcDialog;
}

class ImportMbcDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportMbcDialog(GuiModel* pGuiModel, MbcRegisterModel* pMbcRegisterModel, QWidget* parent = nullptr);
    ~ImportMbcDialog();

public slots:
    int exec(void);

private slots:
    void updateTextFilter();
    void selectMbcFile();
    void visibleItemsDataChanged();
    void registerDataChanged();
    void handleSelectAllClicked(Qt::CheckState state);

private:
    void setSelectedSelectionstate(Qt::CheckState state);
    void updateMbcRegisters(QString filePath);

    Ui::ImportMbcDialog *_pUi;

    CenteredBoxProxyStyle _centeredBoxStyle;

    GuiModel * _pGuiModel;
    MbcRegisterModel * _pMbcRegisterModel;

    MbcRegisterFilter * _pTabProxyFilter;
};

#endif // IMPORTMBCDIALOG_H
