
#include <QObject>

class TestSettingsAuto: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void loadDataFullSample();
    void loadDataFullSampleLimited();

private:





    static QString _cModbusScopeOldFormat;
};
