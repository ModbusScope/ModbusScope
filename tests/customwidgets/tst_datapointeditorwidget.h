#ifndef TST_DATAPOINTEDITORWIDGET_H
#define TST_DATAPOINTEDITORWIDGET_H

#include <QJsonObject>
#include <QObject>

class DataPointEditorWidget;
class DataPointService;
class MockAdapterHub;
class MockDataPointManager;
class SettingsModel;

class TestDataPointEditorWidget : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void buildsDefaultReferenceOnOpen();
    void builtReferenceShownAsPreview();
    void fieldChangeBuildsReference();
    void staleReplyIgnored();
    void deviceSwitchRebuildsForm();
    void setReferenceKeepsTextUntilFieldChange();
    void setReferenceIgnoresUnknownFieldsAndFillsDefaults();
    void setReferenceSelectsDevice();
    void adapterNotRunningShowsError();
    void buildFailureShowsError();
    void emptyBuildResultShowsError();
    void timeoutShowsError();
    void fieldChangeAfterFailureSendsNewRequest();
    void noDevicesHasNoExpression();
    void deviceSwitchClearsPreviousReference();
    void setReferenceWithUnknownDeviceIsInvalid();
    void retriedWhenAdapterBecomesUsable();
    void rejectedBuildKeptWhenAdaptersChange();

private:
    void createEditor();
    void editField();
    static QJsonObject modbusSchema();
    static QJsonObject simSchema();

    SettingsModel* _pSettingsModel{ nullptr };
    MockAdapterHub* _pMockHub{ nullptr };
    MockDataPointManager* _pModbusManager{ nullptr };
    MockDataPointManager* _pSimManager{ nullptr };
    DataPointService* _pService{ nullptr };
    DataPointEditorWidget* _pEditor{ nullptr };
};

#endif // TST_DATAPOINTEDITORWIDGET_H
