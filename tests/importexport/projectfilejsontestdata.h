
#ifndef PROJECTFILEJSONTESTDATA_H
#define PROJECTFILEJSONTESTDATA_H

#include <QString>

class ProjectFileJsonTestData
{
public:
    static QString cTooLowVersion;
    static QString cTooHighVersion;
    static QString cMissingVersion;

    static QString cAdaptersAndDevices;
    static QString cMultipleAdaptersAndDevices;
    static QString cAdapterOnlyNoDevices;

    static QString cLogSettings;
    static QString cLogToFileWithPath;
    static QString cScopeRegisters;
    static QString cViewScaleSliding;
    static QString cViewScaleMinMax;
    static QString cViewScaleWindowAuto;
    static QString cViewScaleAuto;
};

#endif // PROJECTFILEJSONTESTDATA_H
