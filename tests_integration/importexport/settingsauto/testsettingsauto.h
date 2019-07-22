
#include <QObject>

#include "settingsauto.h"

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
    void processDatasetNoLabel();

private:

    bool processFile(QString* pData, SettingsAuto::settingsData_t* pResultData);



    static const qint32 _cSampleLength = 50;
};
