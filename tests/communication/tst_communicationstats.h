
#include <QObject>

/* Forward declaration */
class GraphDataModel;
class SettingsModel;
class CommunicationStats;

class TestCommunicationStats: public QObject
{
    Q_OBJECT
    
private slots:
    void init();
    void cleanup();

    void noGraph();
    void notEnoughDataCount();
    void justEnoughDataCount();
    void dataCountIsEven();
    void dataCountIsNotEven();
    void pollTimeDiffIsGettingSmaller();
    void onlyLastXSamples();

private:
    void setPollData(QVector<double> times);

    SettingsModel* _pSettingsModel;
    GraphDataModel* _pGraphDataModel;

    CommunicationStats* _pCommunicationStats;

};