#ifndef PROJECTFILEDEFINITIONS
#define PROJECTFILEDEFINITIONS

#include <QString>

namespace ProjectFileDefinitions
{

    /* Tag strings */
    const QString cModbusScopeTag = QString("modbusscope");
    const QString cModbusTag = QString("modbus");
    const QString cScopeTag = QString("scope");
    const QString cViewTag = QString("view");
    const QString cConnectionTag = QString("connection");
    const QString cLogTag = QString("log");
    const QString cIpTag = QString("ip");
    const QString cSlaveIdTag = QString("slaveid");
    const QString cPortTag = QString("port");
    const QString cTimeoutTag = QString("timeout");
    const QString cConsecutiveMaxTag = QString("consecutivemax");
    const QString cPollTimeTag = QString("polltime");
    const QString cAbsoluteTimesTag = QString("absolutetimes");
    const QString cLogToFileTag = QString("logtofile");
    const QString cFilenameTag = QString("filename");
    const QString cRegisterTag = QString("register");
    const QString cAddressTag = QString("address");
    const QString cTextTag = QString("text");
    const QString cUnsignedTag = QString("unsigned");
    const QString cDivideTag = QString("divide");
    const QString cMultiplyTag = QString("multiply");
    const QString cColorTag = QString("color");
    const QString cBitmaskTag = QString("bitmask");
    const QString cShiftTag = QString("shift");
    const QString cScaleTag = QString("scale");
    const QString cXaxisTag = QString("xaxis");
    const QString cYaxisTag = QString("yaxis");
    const QString cMinTag = QString("min");
    const QString cMaxTag = QString("max");
    const QString cSlidingintervalTag = QString("slidinginterval");

    /* Attribute string */
    const QString cDatalevelAttribute = QString("datalevel");
    const QString cEnabledAttribute = QString("enabled");
    const QString cActiveAttribute = QString("active");
    const QString cModeAttribute = QString("mode");

    /* Value strings */
    const QString cSlidingValue = QString("sliding");
    const QString cAutoValue = QString("auto");
    const QString cMinmaxValue = QString("minmax");
    const QString cTrueValue = QString("true");
    const QString cFalseValue = QString("false");

    /* Constant values */
    const quint32 cCurrentDataLevel = 2;

}

#endif // PROJECTFILEDEFINITIONS

