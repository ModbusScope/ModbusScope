#ifndef ADDREGISTERWIDGET_H
#define ADDREGISTERWIDGET_H

#include "models/graphdata.h"

#include <QButtonGroup>
#include <QWidget>

class DataPointEditorWidget;
class DataPointService;
class SettingsModel;

namespace Ui {
class AddRegisterWidget;
}

class AddRegisterWidget : public QWidget
{
    Q_OBJECT

    friend class TestAddRegisterWidget;

public:
    explicit AddRegisterWidget(SettingsModel* pSettingsModel,
                               DataPointService* pDataPointService,
                               QWidget* parent = nullptr);
    ~AddRegisterWidget();

signals:
    void graphDataConfigured(GraphData graphData);

private slots:
    void handleAdd();
    void onExpressionChanged();
    void setBuildError(const QString& error);
    void resizeContainingMenu();
    void closeContainingMenu();

private:
    void resetFields();

    Ui::AddRegisterWidget* _pUi;
    DataPointEditorWidget* _pEditor;

    QButtonGroup _axisGroup;
};

#endif // ADDREGISTERWIDGET_H
