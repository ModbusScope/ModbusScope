#ifndef PROJECTFILEDATA
#define PROJECTFILEDATA

#include "models/device.h"
#include <QColor>
#include <QJsonObject>
#include <QList>

namespace ProjectFileData {

typedef struct _AdapterFileSettings
{
    QString type;         //!< Adapter type identifier, e.g. "modbus"
    QJsonObject settings; //!< Opaque settings blob passed as-is to the adapter

} AdapterFileSettings;

typedef struct _RegisterSettings
{
    QString text = QString("");
    bool bActive = false;

    QString expression = QString("0");

    bool bColor = false;
    QColor color;

    quint32 valueAxis = 0;

} RegisterSettings;

typedef struct
{
    QList<RegisterSettings> registerList;
} ScopeSettings;

typedef struct _XAxisSettings
{
    bool bSliding = false;
    quint32 slidingInterval = 60;

} XAxisSettings;

typedef struct _YAxisSettings
{
    bool bWindowScale = false;

    bool bMinMax = false;
    double scaleMin = 0;
    double scaleMax = 10;
} YAxisSettings;

typedef struct _ScaleSettings
{
    XAxisSettings xAxis;
    YAxisSettings yAxis;
    YAxisSettings y2Axis;
} ScaleSettings;

typedef struct
{
    ScaleSettings scaleSettings;

} ViewSettings;

typedef struct _LogSettings
{
    bool bPollTime = false;
    quint32 pollTime;

    bool bAbsoluteTimes = false;

    bool bLogToFile = true;
    bool bLogToFileFile = false;
    QString logFile;

} LogSettings;

typedef struct _DeviceSettings
{
    bool bDeviceId = false;
    deviceId_t deviceId = Device::cFirstDeviceId;

    bool bName = false;
    QString name = QString("");

    /* JSON format fields */
    quint32 adapterId = 0; //!< Index into GeneralSettings::adapterList
    QString adapterType;   //!< Quick-lookup copy of adapters[adapterId].type

} DeviceSettings;

typedef struct _GeneralSettings
{
    QList<DeviceSettings> deviceSettings;
    LogSettings logSettings;

    /* JSON format: populated by ProjectFileJsonParser */
    QList<AdapterFileSettings> adapterList;

} GeneralSettings;

typedef struct
{
    GeneralSettings general;
    ScopeSettings scope;
    ViewSettings view;
} ProjectSettings;
} // namespace ProjectFileData

#endif // PROJECTFILEDATA
