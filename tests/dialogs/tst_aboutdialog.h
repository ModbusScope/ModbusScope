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

    void installLicenseFileCopiesToFreshDestination();
    void installLicenseFileCreatesMissingDirectories();
    void installLicenseFileOverwritesExistingDestination();
    void installLicenseFileFailsWhenSourceMissing();
    void installLicenseFileHandlesSourceEqualsDestination();
};

#endif // TST_ABOUTDIALOG_H
