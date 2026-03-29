#ifndef PROJECTFILEDEFINITIONS_H
#define PROJECTFILEDEFINITIONS_H

#include <QString>

namespace ProjectFileDefinitions {

/* Tag strings */
const char cModbusScopeTag[] = "modbusscope";
const char cModbusTag[] = "modbus";
const char cScopeTag[] = "scope";
const char cViewTag[] = "view";
const char cDeviceTag[] = "device";
const char cDeviceIdTag[] = "deviceid";
const char cDeviceNameTag[] = "name";
const char cLogTag[] = "log";
const char cPollTimeTag[] = "polltime";
const char cAbsoluteTimesTag[] = "absolutetimes";
const char cLogToFileTag[] = "logtofile";
const char cFilenameTag[] = "filename";
const char cRegisterTag[] = "register";
const char cTextTag[] = "text";
const char cExpressionTag[] = "expression";
const char cColorTag[] = "color";
const char cValueAxisTag[] = "valueaxis";

const char cScaleTag[] = "scale";
const char cXaxisTag[] = "xaxis";
const char cYaxisTag[] = "yaxis";
const char cMinTag[] = "min";
const char cMaxTag[] = "max";
const char cSlidingintervalTag[] = "slidinginterval";

/* Attribute string */
const char cDatalevelAttribute[] = "datalevel";
const char cEnabledAttribute[] = "enabled";
const char cActiveAttribute[] = "active";
const char cModeAttribute[] = "mode";
const char cAxisAttribute[] = "axis";

/* Value strings */
const char cSlidingValue[] = "sliding";
const char cAutoValue[] = "auto";
const char cMinmaxValue[] = "minmax";
const char cWindowAutoValue[] = "windowauto";
const char cTrueValue[] = "true";
const char cFalseValue[] = "false";

/* XML constant values */
const quint32 cMinimumDataLevel = 3;
const quint32 cCurrentDataLevel = 5;

/* JSON-specific keys */
const char cVersionKey[] = "version";
const char cAdaptersKey[] = "adapters";
const char cAdapterTypeKey[] = "type";
const char cAdapterSettingsKey[] = "settings";
const char cAdapterIdKey[] = "adapterId";
const char cAdapterKey[] = "adapter";
const char cIdJsonKey[] = "id";
const char cDevicesJsonKey[] = "devices";

/* JSON constant values */
const quint32 cMinimumJsonVersion = 6;
const quint32 cCurrentJsonVersion = 6;
} // namespace ProjectFileDefinitions

#endif // PROJECTFILEDEFINITIONS_H
