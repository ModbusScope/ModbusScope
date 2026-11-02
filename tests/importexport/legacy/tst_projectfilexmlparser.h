
#ifndef TST_PROJECTFILEXMLPARSER_H
#define TST_PROJECTFILEXMLPARSER_H

#include <QObject>

class TestProjectFileXmlParser : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void initTestCase();
    void cleanup();

    void tooLowDataLevel();
    void tooHighDataLevel();

    void dataLevel3Expressions();

    void dataLevel5Connection();
    void dataLevel5MixedMulti();
    void dataLevel4LegacyConnection();
    void dataLevel4LegacyConnectionsWithoutConnectionIdCollide();

    void connectionUsesAdapterKeys_data();
    void connectionUsesAdapterKeys();
    void deviceConnectionIdMatchesConnection_data();
    void deviceConnectionIdMatchesConnection();
    void connectionIdsStartAtOne();
    void parityMapsToLetters_data();
    void parityMapsToLetters();
    void parityMissingDefaultsToNone();
    void invalidNumberIsParseError_data();
    void invalidNumberIsParseError();
    void stopbitsAndDatabitsKept();
    void outOfRangeValuesUseDefaults();
    void outOfRangeDeviceValuesUseDefaults_data();
    void outOfRangeDeviceValuesUseDefaults();
    void typeDefaultsToTcp_data();
    void typeDefaultsToTcp();
    void missingFieldsUseOldDefaults();
    void connectionNameFromId();
    void disabledUnusedConnectionDropped_data();
    void disabledUnusedConnectionDropped();
    void disabledReferencedConnectionKept();
    void duplicateConnectionIdsKeepFirst();
    void deviceIdsUnchanged();
    void demoDataLevel4ParsesCompletely();
    void demoDataLevel5ParsesCompletely();

    void logSettings();

    void scaleSliding();
    void scaleMinMax();
    void scaleWindowAuto();

    void valueAxis();
    void valueAxisInvalid();

    void logFileRelativePath();
};

#endif // TST_PROJECTFILEXMLPARSER_H
