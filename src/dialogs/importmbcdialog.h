#ifndef IMPORTMBCDIALOG_H
#define IMPORTMBCDIALOG_H

#include <QDialog>

/* Forward declaration */
class GuiModel;

namespace Ui {
class ImportMbcDialog;
}

class ImportMbcDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportMbcDialog(GuiModel * pGuiModel, QWidget *parent = 0);
    ~ImportMbcDialog();

public slots:
    int exec();

private slots:
    void selectMbcFile();

private:

    bool updateMbcRegisters();

    Ui::ImportMbcDialog *_pUi;

    GuiModel * _pGuiModel;

    QString _mbcFilePath;
};

#endif // IMPORTMBCDIALOG_H
