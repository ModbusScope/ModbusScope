
#ifndef TST_ADDREGISTERWIDGET_H
#define TST_ADDREGISTERWIDGET_H

#include "communication/modbuspoll.h"
#include "models/graphdata.h"
#include "models/settingsmodel.h"

#include <QJsonObject>
#include <QObject>

class AddRegisterWidget;

/*!
 * \brief Test double for ModbusPoll.
 *
 * Captures buildExpression() calls and provides an inject helper to simulate
 * the adapter.buildExpression response without a real adapter process.
 */
class MockModbusPoll : public ModbusPoll
{
    Q_OBJECT

public:
    struct BuildCall
    {
        QJsonObject fields;
        QString dataType;
        deviceId_t deviceId;
    };

    explicit MockModbusPoll(SettingsModel* pSettingsModel, QObject* parent = nullptr)
        : ModbusPoll(pSettingsModel, parent)
    {
    }

    void buildExpression(const QJsonObject& fields, const QString& dataType, deviceId_t deviceId) override
    {
        buildCalls.append({ fields, dataType, deviceId });
    }

    //! Simulate the adapter returning an expression string.
    void injectBuildExpressionResult(const QString& expression)
    {
        emit buildExpressionResult(expression);
    }

    QList<BuildCall> buildCalls;
};

class TestAddRegisterWidget : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void registerDefault();
    void registerType();
    void registerObjectType();
    void registerDevice();
    void registerValueAxis();
    void buildExpressionEmptyResponseIgnored();

private:
    void clickAdd();
    void addRegister(GraphData& graphData, const QString& expression);
    static QJsonObject buildAddressSchema();
    static QJsonObject buildTestRegisterSchema();

    SettingsModel _settingsModel;
    MockModbusPoll* _pMockModbusPoll{ nullptr };
    AddRegisterWidget* _pRegWidget{ nullptr };
};

#endif // TST_ADDREGISTERWIDGET_H
