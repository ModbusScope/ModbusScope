#ifndef TST_MBCFILEIMPORTER_H
#define TST_MBCFILEIMPORTER_H

#include "importexport/mbcregisterdata.h"

#include <QObject>

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

    void importInvalidXml();
    void importWrongRootTag();
    void importMissingName();
    void importMissingAddress();
    void importMissingRw();
    void importWriteOnlyRegister();
    void importUnknownType();
    void importMixedReadWriteRegisters();
    void importEmptyVarTag();

private:
    void verifyRegList(QList <MbcRegisterData> list1, QList <MbcRegisterData> list2);
};

#endif // TST_MBCFILEIMPORTER_H
