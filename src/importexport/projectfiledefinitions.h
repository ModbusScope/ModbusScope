#ifndef PROJECTFILEDEFINITIONS
#define PROJECTFILEDEFINITIONS

#include <QString>

namespace ProjectFileDefinitions
{

    /* Tag strings */
    const char cModbusScopeTag[] = "modbusscope";
    const char cModbusTag[] = "modbus";
    const char cScopeTag[] = "scope";
    const char cViewTag[] = "view";
    const char cConnectionTag[] = "connection";
    const char cLogTag[] = "log";
    const char cIpTag[] = "ip";
    const char cConnectionIdTag[] = "connectionid";
    const char cSlaveIdTag[] = "slaveid";
    const char cPortTag[] = "port";
    const char cConnectionEnabledTag[] = "enabled";
    const char cConnectionTypeTag[] = "type";
    const char cPortNameTag[] = "portname";
    const char cParityTag[] = "parity";
    const char cBaudrateTag[] = "baudrate";
    const char cDataBitsTag[] = "databits";
    const char cStopBitsTag[] = "stopbits";
    const char cTimeoutTag[] = "timeout";
    const char cConsecutiveMaxTag[] = "consecutivemax";
    const char cInt32LittleEndianTag[] = "int32littleendian";
    const char cPersistentConnectionTag[] = "persistentconnection";
    const char cPollTimeTag[] = "polltime";
    const char cAbsoluteTimesTag[] = "absolutetimes";
    const char cLogToFileTag[] = "logtofile";
    const char cFilenameTag[] = "filename";
    const char cRegisterTag[] = "register";
    const char cAddressTag[] = "address";
    const char cTextTag[] = "text";
    const char cUnsignedTag[] = "unsigned";
    const char cDivideTag[] = "divide";
    const char cBit32Tag[] = "bit32";
    const char cMultiplyTag[] = "multiply";
    const char cExpressionTag[] = "expression";
    const char cColorTag[] = "color";
    const char cValueAxisTag[] = "valueaxis";
    const char cBitmaskTag[] = "bitmask";
    const char cShiftTag[] = "shift";

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

    /* Value strings */
    const char cSlidingValue[] = "sliding";
    const char cAutoValue[] = "auto";
    const char cMinmaxValue[] = "minmax";
    const char cWindowAutoValue[] = "windowauto";
    const char cTrueValue[] = "true";
    const char cFalseValue[] = "false";

    /* Constant values */
    const quint32 cCurrentDataLevel = 3;

}

#endif // PROJECTFILEDEFINITIONS

