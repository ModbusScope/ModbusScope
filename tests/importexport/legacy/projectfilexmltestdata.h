#ifndef PROJECTFILEXMLTESTDATA_H
#define PROJECTFILEXMLTESTDATA_H

#include <QString>

class ProjectFileXmlTestData
{
public:
    static QString cTooLowDataLevel;
    static QString cTooHighDataLevel;

    static QString cDataLevel3Expressions;
    static QString cDataLevel5Connection;
    static QString cDataLevel5MixedMulti;
    static QString cDataLevel4LegacyConnection;
    static QString cDataLevel4LegacyConnectionNoIds;
    static QString cDataLevel3Connections;
    static QString cDataLevel4Demo;
    static QString cDataLevel5Demo;
    static QString cSerialVariants;
    static QString cOutOfRangeValues;
    static QString cMinimalConnection;
    static QString cDisabledReferencedConnection;

    static QString cLogSettings;
    static QString cScaleSliding;
    static QString cScaleMinMax;
    static QString cScaleWindowAuto;
    static QString cValueAxis;
    static QString cValueAxisInvalid;
    static QString cLogFileRelativePath;
};

#endif // PROJECTFILEXMLTESTDATA_H
