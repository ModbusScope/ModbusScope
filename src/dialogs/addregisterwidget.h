#ifndef ADDREGISTERWIDGET_H
#define ADDREGISTERWIDGET_H

#include "models/device.h"
#include "models/graphdata.h"

#include <QButtonGroup>
#include <QJsonObject>
#include <QWidget>

class DataPointService;
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
                               DataPointService* pDataPointService,
                               QWidget* parent = nullptr);
    ~AddRegisterWidget();

signals:
    void graphDataConfigured(GraphData graphData);

private slots:
    void handleResultAccept();
    void onExpressionBuilt(quint64 ticket, const QString& expression);
    void onBuildRequestFailed(quint64 ticket, const QString& error);
    void applyDevice(deviceId_t deviceId);

private:
    void resetFields();
    void collectPendingGraphData();
    void rebuildAddressForm();
    void setBuildError(const QString& error);
    void showBuildErrorForPendingDevice(const QString& error);
    void resizeContainingMenu();
    void closeContainingMenu();
    bool isSelectionUsable() const;

    Ui::AddRegisterWidget* _pUi;
    SchemaFormWidget* _pAddressForm;
    QJsonObject _addressSchema;
    QJsonObject _dataPointDefaults;

    SettingsModel* _pSettingsModel;
    DataPointService* _pDataPointService;

    QButtonGroup _axisGroup;

    /* Temporary storage while waiting for buildExpression response */
    GraphData _pendingGraphData;
    quint64 _pendingBuildTicket{ 0 };
    deviceId_t _pendingBuildDeviceId{ 0 };
};

#endif // ADDREGISTERWIDGET_H
