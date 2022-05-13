
#include <QObject>

#include "mbcfileimporter.h"

class TestMbcFileImporter: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void importSingleTab();
    void importMultiTab();
    void importRegisterOptions();
    void importAutoIncrement();

private:
    void verifyRegList(QList <MbcRegisterData> list1, QList <MbcRegisterData> list2);
};
