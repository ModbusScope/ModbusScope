#ifndef ADDREGISTERWIDGET_H
#define ADDREGISTERWIDGET_H

#include "models/device.h"
#include "models/graphdata.h"

#include <QButtonGroup>
#include <QJsonObject>
#include <QWidget>

class ModbusPoll;
class SchemaFormWidget;
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
                               const QString& adapterId,
                               ModbusPoll* pModbusPoll,
                               QWidget* parent = nullptr);
    ~AddRegisterWidget();

signals:
    void graphDataConfigured(GraphData graphData);

private slots:
    void handleResultAccept();
    void onBuildExpressionResult(const QString& expression);

private:
    void resetFields();
    void collectPendingGraphData();

    Ui::AddRegisterWidget* _pUi;
    SchemaFormWidget* _pAddressForm;
    QJsonObject _addressSchema;
    int _defaultTypeIndex{ 0 };

    SettingsModel* _pSettingsModel;
    ModbusPoll* _pModbusPoll;

    QButtonGroup _axisGroup;

    /* Temporary storage while waiting for buildExpression response */
    GraphData _pendingGraphData;
};

#endif // ADDREGISTERWIDGET_H
