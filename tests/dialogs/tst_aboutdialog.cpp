#include "tst_aboutdialog.h"

#include "dialogs/aboutdialog.h"
#include "models/adapterdata.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

void TestAboutDialog::requiresOverwriteConfirmationTrueForValid()
{
    AdapterLicenseInfo license;
    license.state = AdapterLicenseInfo::State::Valid;

    QVERIFY(AboutDialog::requiresOverwriteConfirmation(license));
}

void TestAboutDialog::requiresOverwriteConfirmationFalseForInvalid()
{
    AdapterLicenseInfo license;
    license.state = AdapterLicenseInfo::State::Invalid;

    QVERIFY(!AboutDialog::requiresOverwriteConfirmation(license));
}

void TestAboutDialog::requiresOverwriteConfirmationFalseForNotFound()
{
    AdapterLicenseInfo license;
    license.state = AdapterLicenseInfo::State::NotFound;

    QVERIFY(!AboutDialog::requiresOverwriteConfirmation(license));
}

void TestAboutDialog::requiresOverwriteConfirmationFalseForUnknown()
{
    AdapterLicenseInfo license;
    license.state = AdapterLicenseInfo::State::Unknown;

    QVERIFY(!AboutDialog::requiresOverwriteConfirmation(license));
}

void TestAboutDialog::installLicenseFileCopiesToFreshDestination()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString sourcePath = tempDir.filePath("source.lic");
    QFile sourceFile(sourcePath);
    QVERIFY(sourceFile.open(QIODevice::WriteOnly));
    sourceFile.write("license-content");
    sourceFile.close();

    const QString destPath = tempDir.filePath("dest.lic");

    const QString error = AboutDialog::installLicenseFile(sourcePath, destPath);

    QVERIFY(error.isEmpty());
    QVERIFY(QFile::exists(destPath));
}

void TestAboutDialog::installLicenseFileCreatesMissingDirectories()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString sourcePath = tempDir.filePath("source.lic");
    QFile sourceFile(sourcePath);
    QVERIFY(sourceFile.open(QIODevice::WriteOnly));
    sourceFile.write("license-content");
    sourceFile.close();

    const QString destPath = tempDir.filePath("nested/licenses/dest.lic");

    const QString error = AboutDialog::installLicenseFile(sourcePath, destPath);

    QVERIFY(error.isEmpty());
    QVERIFY(QFile::exists(destPath));
}

void TestAboutDialog::installLicenseFileOverwritesExistingDestination()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString sourcePath = tempDir.filePath("source.lic");
    QFile sourceFile(sourcePath);
    QVERIFY(sourceFile.open(QIODevice::WriteOnly));
    sourceFile.write("new-content");
    sourceFile.close();

    const QString destPath = tempDir.filePath("dest.lic");
    QFile destFile(destPath);
    QVERIFY(destFile.open(QIODevice::WriteOnly));
    destFile.write("old-content");
    destFile.close();

    const QString error = AboutDialog::installLicenseFile(sourcePath, destPath);

    QVERIFY(error.isEmpty());
    QFile writtenFile(destPath);
    QVERIFY(writtenFile.open(QIODevice::ReadOnly));
    QCOMPARE(writtenFile.readAll(), QByteArray("new-content"));
}

void TestAboutDialog::installLicenseFileFailsWhenSourceMissing()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString sourcePath = tempDir.filePath("does-not-exist.lic");
    const QString destPath = tempDir.filePath("dest.lic");

    const QString error = AboutDialog::installLicenseFile(sourcePath, destPath);

    QVERIFY(!error.isEmpty());
    QVERIFY(!QFile::exists(destPath));
}

void TestAboutDialog::installLicenseFileHandlesSourceEqualsDestination()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString path = tempDir.filePath("license.lic");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("existing-content");
    file.close();

    const QString error = AboutDialog::installLicenseFile(path, path);

    QVERIFY(error.isEmpty());
    QFile writtenFile(path);
    QVERIFY(writtenFile.open(QIODevice::ReadOnly));
    QCOMPARE(writtenFile.readAll(), QByteArray("existing-content"));
}

QTEST_MAIN(TestAboutDialog)
