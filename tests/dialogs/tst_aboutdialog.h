#ifndef TST_ABOUTDIALOG_H
#define TST_ABOUTDIALOG_H

#include <QObject>

class TestAboutDialog : public QObject
{
    Q_OBJECT

private slots:
    void requiresOverwriteConfirmationTrueForValid();
    void requiresOverwriteConfirmationFalseForInvalid();
    void requiresOverwriteConfirmationFalseForNotFound();
    void requiresOverwriteConfirmationFalseForUnknown();

    void licensePreviewTextShowsAllFields();
    void licensePreviewTextEscapesHtml();
    void licensePreviewTextOmitsOptionalFields();
    void licensePreviewTextMentionsReplacedLicense();
    void licenseRejectionTextInvalidShowsReason();
    void licenseRejectionTextNotFound();
    void licenseRejectionTextEscapesHtml();

    void installLicenseFileCopiesToFreshDestination();
    void installLicenseFileCreatesMissingDirectories();
    void installLicenseFileOverwritesExistingDestination();
    void installLicenseFileFailsWhenSourceMissing();
    void installLicenseFileHandlesSourceEqualsDestination();
};

#endif // TST_ABOUTDIALOG_H
