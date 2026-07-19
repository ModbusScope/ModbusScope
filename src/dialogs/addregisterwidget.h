#ifndef ADDREGISTERWIDGET_H
#define ADDREGISTERWIDGET_H

#include "models/device.h"
#include "models/graphdata.h"

#include <QButtonGroup>
#include <QJsonObject>
#include <QWidget>

class AdapterHub;
class AdapterManager;
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
    explicit AddRegisterWidget(SettingsModel* pSettingsModel, AdapterHub* pAdapterHub, QWidget* parent = nullptr);
    ~AddRegisterWidget();

signals:
    void graphDataConfigured(GraphData graphData);

private slots:
    void handleResultAccept();
    void onBuildExpressionResult(const QString& expression);
    void onAdapterSelectionChanged(int index);

private:
    void resetFields();
    void collectPendingGraphData();
    void applyAdapter(const QString& adapterId);
    void rebuildAddressForm();
    bool isAdapterUsable(const QString& adapterId) const;
    QString selectedAdapterId() const;
    QJsonObject buildSchema(const QString& adapterId) const;

    Ui::AddRegisterWidget* _pUi;
    SchemaFormWidget* _pAddressForm;
    QJsonObject _addressSchema;
    QJsonObject _dataPointDefaults;

    SettingsModel* _pSettingsModel;
    AdapterHub* _pAdapterHub;
    AdapterManager* _pAdapterManager;

    QButtonGroup _axisGroup;

    /* Temporary storage while waiting for buildExpression response */
    GraphData _pendingGraphData;
};

#endif // ADDREGISTERWIDGET_H
