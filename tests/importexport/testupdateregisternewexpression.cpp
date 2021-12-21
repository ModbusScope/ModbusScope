
#include <QtTest/QtTest>

#include "updateregisternewexpression.h"

#include "testupdateregisternewexpression.h"


void TestUpdateRegisterNewExpression::init()
{

}

void TestUpdateRegisterNewExpression::cleanup()
{

}

void TestUpdateRegisterNewExpression::tags()
{
    ProjectFileData::RegisterSettings regSettings;

    regSettings.address = 40001;
    regSettings.b32Bit = false;
    regSettings.bUnsigned = true;
    regSettings.connectionId = 0;

    checkOperation(regSettings, QStringLiteral("${40001}"));
}

void TestUpdateRegisterNewExpression::tags_signed32()
{
    ProjectFileData::RegisterSettings regSettings;

    regSettings.address = 40002;
    regSettings.b32Bit = true;
    regSettings.bUnsigned = false;
    regSettings.connectionId = 0;

    checkOperation(regSettings, QStringLiteral("${40002:s32b}"));
}

void TestUpdateRegisterNewExpression::tags_signed16()
{
    ProjectFileData::RegisterSettings regSettings;

    regSettings.address = 40002;
    regSettings.b32Bit = false;
    regSettings.bUnsigned = false;
    regSettings.connectionId = 0;

    checkOperation(regSettings, QStringLiteral("${40002:s16b}"));
}

void TestUpdateRegisterNewExpression::tags_connection()
{
    ProjectFileData::RegisterSettings regSettings;

    regSettings.address = 40002;
    regSettings.b32Bit = false;
    regSettings.bUnsigned = true;
    regSettings.connectionId = 1;

    checkOperation(regSettings, QStringLiteral("${40002@2}"));
}

void TestUpdateRegisterNewExpression::tags_signed16_connection()
{
    ProjectFileData::RegisterSettings regSettings;

    regSettings.address = 40002;
    regSettings.b32Bit = false;
    regSettings.bUnsigned = false;
    regSettings.connectionId = 1;

    checkOperation(regSettings, QStringLiteral("${40002@2:s16b}"));
}

void TestUpdateRegisterNewExpression::checkOperation(ProjectFileData::RegisterSettings& regSettings, QString regExpr)
{
    QString regResultExpr;

    UpdateRegisterNewExpression::convert(regSettings, regResultExpr);

    QCOMPARE(regResultExpr, regExpr);
}


QTEST_GUILESS_MAIN(TestUpdateRegisterNewExpression)
