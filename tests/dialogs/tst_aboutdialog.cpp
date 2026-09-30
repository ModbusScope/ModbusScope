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

void TestAboutDialog::licensePreviewTextShowsAllFields()
{
    AdapterLicenseInfo license;
    license.state = AdapterLicenseInfo::State::Valid;
    license.customer = "ACME Corp";
    license.email = "customer@example.com";
    license.licenseId = "LIC-2026-001";
    license.expires = "2027-01-01";

    const QString text = AboutDialog::licensePreviewText(license, false);

    QVERIFY(text.contains("ACME Corp"));
    QVERIFY(text.contains("customer@example.com"));
    QVERIFY(text.contains("LIC-2026-001"));
    QVERIFY(text.contains("2027-01-01"));
    QVERIFY(text.contains("issued to you or your organisation"));
}

void TestAboutDialog::licensePreviewTextEscapesHtml()
{
    AdapterLicenseInfo license;
    license.state = AdapterLicenseInfo::State::Valid;
    license.customer = "<b>Evil</b> & Co";
    license.email = "<x@y.z>";
    license.licenseId = "<i>1</i>";
    license.expires = "<u>never</u>";

    const QString text = AboutDialog::licensePreviewText(license, false);

    QVERIFY(!text.contains("<b>Evil</b>"));
    QVERIFY(!text.contains("<i>1</i>"));
    QVERIFY(!text.contains("<u>never</u>"));
    QVERIFY(text.contains("&lt;b&gt;Evil&lt;/b&gt; &amp; Co"));
}

void TestAboutDialog::licensePreviewTextOmitsOptionalFields()
{
    AdapterLicenseInfo license;
    license.state = AdapterLicenseInfo::State::Valid;
    license.customer = "ACME Corp";
    license.licenseId = "LIC-2026-001";

    const QString text = AboutDialog::licensePreviewText(license, false);

    QVERIFY(text.contains("ACME Corp"));
    QVERIFY(text.contains("LIC-2026-001"));
    QVERIFY(!text.contains("Email"));
    QVERIFY(!text.contains("Expires"));
}

void TestAboutDialog::licensePreviewTextMentionsReplacedLicense()
{
    AdapterLicenseInfo license;
    license.state = AdapterLicenseInfo::State::Valid;
    license.customer = "ACME Corp";
    license.licenseId = "LIC-2026-001";

    const QString withReplace = AboutDialog::licensePreviewText(license, true);
    const QString withoutReplace = AboutDialog::licensePreviewText(license, false);

    QVERIFY(withReplace.contains("replace"));
    QVERIFY(!withoutReplace.contains("replace"));
}

void TestAboutDialog::licenseRejectionTextInvalidShowsReason()
{
    AdapterLicenseInfo license;
    license.state = AdapterLicenseInfo::State::Invalid;
    license.reason = "license expired";

    const QString text = AboutDialog::licenseRejectionText(license);

    QVERIFY(text.contains("license expired"));
}

void TestAboutDialog::licenseRejectionTextNotFound()
{
    AdapterLicenseInfo license;
    license.state = AdapterLicenseInfo::State::NotFound;

    const QString text = AboutDialog::licenseRejectionText(license);

    QVERIFY(!text.isEmpty());
    QVERIFY(text.contains("not found", Qt::CaseInsensitive));
}

void TestAboutDialog::licenseRejectionTextEscapesHtml()
{
    AdapterLicenseInfo license;
    license.state = AdapterLicenseInfo::State::Invalid;
    license.reason = "<b>bad</b>";

    const QString text = AboutDialog::licenseRejectionText(license);

    QVERIFY(!text.contains("<b>bad</b>"));
    QVERIFY(text.contains("&lt;b&gt;bad&lt;/b&gt;"));
}

void TestAboutDialog::licenseRejectionTextUnknownAndEmptyReason()
{
    AdapterLicenseInfo unknown;
    QVERIFY(AboutDialog::licenseRejectionText(unknown).contains("unrecognised"));

    AdapterLicenseInfo invalid;
    invalid.state = AdapterLicenseInfo::State::Invalid;
    QVERIFY(!AboutDialog::licenseRejectionText(invalid).endsWith(": "));
    QVERIFY(!AboutDialog::licenseRejectionText(invalid).endsWith(":"));
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
