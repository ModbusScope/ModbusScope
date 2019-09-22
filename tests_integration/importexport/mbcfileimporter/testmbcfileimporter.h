
#include <QObject>

class TestMbcFileImporter: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void importSingleTab();

private:

};
