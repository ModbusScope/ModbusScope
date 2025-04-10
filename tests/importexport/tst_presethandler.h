
#include <QObject>

class TestPresetHandler: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void determinePresetEmptyList();
    void determinePresetFail();
    void determinePresetSuccess();

    void emptyNameList();
    void nameList();

private:

};
