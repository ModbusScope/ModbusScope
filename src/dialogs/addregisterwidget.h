#ifndef ADDREGISTERWIDGET_H
#define ADDREGISTERWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include "graphdata.h"

class SettingsModel;

namespace Ui {
class AddRegisterWidget;
}

class AddRegisterWidget : public QWidget
{
    Q_OBJECT

    friend class TestAddRegisterWidget;

public:
    explicit AddRegisterWidget(SettingsModel* pSettingsModel, QWidget *parent = nullptr);
    ~AddRegisterWidget();

signals:
    void graphDataConfigured(GraphData graphData);

private slots:
    void handleResultAccept();

private:
    void resetFields();
    QString generateExpression();

    Ui::AddRegisterWidget * _pUi;

    SettingsModel* _pSettingsModel;

    QButtonGroup _axisGroup;
};

#endif // ADDREGISTERWIDGET_H
