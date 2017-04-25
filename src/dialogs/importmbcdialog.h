#ifndef IMPORTMBCDIALOG_H
#define IMPORTMBCDIALOG_H

#include <QDialog>

/* Forward declaration */
class GuiModel;
class GraphData;
class GraphDataModel;

namespace Ui {
class ImportMbcDialog;
}

class ImportMbcDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportMbcDialog(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel, QWidget *parent = 0);
    ~ImportMbcDialog();

    void selectedRegisterList(QList<GraphData> * pRegList);

public slots:
    int exec(void);
    int exec(QString mbcPath);

private slots:
    void selectMbcFile();

private:

    bool updateMbcRegisters();

    Ui::ImportMbcDialog *_pUi;

    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;

    QString _mbcFilePath;
};

#endif // IMPORTMBCDIALOG_H
