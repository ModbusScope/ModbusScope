
#include <QObject>

class TestPresetFileParser: public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void singlePreset();
    void invalidFile();

private:

};
