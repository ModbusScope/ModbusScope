#ifndef IMPORTMBCDIALOG_H
#define IMPORTMBCDIALOG_H

#include <QDialog>
#include <qtablewidget.h>

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
    explicit ImportMbcDialog(GuiModel * pGuiModel, GraphDataModel * pGraphDataModel, QWidget *parent = nullptr);
    ~ImportMbcDialog();

    QList<GraphData> selectedRegisterList(void);

public slots:
    int exec(void);
    int exec(QString mbcPath);

private slots:
    void selectMbcFile();
    void registerSelectionChanged(QTableWidgetItem * pItem);
    void tabFilterChanged(const QString &text);

private:

    bool updateMbcRegisters();
    void updateSelectedRegisters();

    QList<GraphData> _selectedRegisterList;

    Ui::ImportMbcDialog *_pUi;

    GuiModel * _pGuiModel;
    GraphDataModel * _pGraphDataModel;

    QString _mbcFilePath;

    static const QString _cTabFilterAll;
};

#endif // IMPORTMBCDIALOG_H
