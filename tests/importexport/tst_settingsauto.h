
#include "importexport/settingsauto.h"

#include <QObject>

class TestSettingsAuto: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void loadDataFullSample();
    void loadDataFullSampleLimited();

    void processOldModbusScope();
    void processNewModbusScope();

    void processDatasetBe();
    void processDatasetUs();

    void processDatasetColumn2();
    void processDatasetComment();

    void processDatasetSigned();

    void processDatasetAbsoluteDate();
    void processDatasetTimeInSeconds();
    void processDatasetExcelChanged();

private:

    void prepareReference(QString* pRefData, QStringList& refList);
    bool processFile(QString* pData, SettingsAuto::settingsData_t* pResultData);

    static const qint32 _cSampleLength = 50;
};
