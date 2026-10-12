#ifndef TST_ADAPTERCONFIGDRAFT_H
#define TST_ADAPTERCONFIGDRAFT_H

#include <QObject>

class TestAdapterConfigDraft : public QObject
{
    Q_OBJECT

private slots:
    void configFallsBackToSavedConfig();
    void sectionOverridesSavedKey();
    void sectionsOfDifferentKeysAreKept();
    void sectionsOfDifferentAdaptersAreIndependent();
    void setSectionEmitsConfigChanged();
    void unchangedSectionDoesNotEmit();
    void modelUpdateEmitsConfigChangedAndKeepsSections();
    void draftDoesNotWriteModel();
};

#endif // TST_ADAPTERCONFIGDRAFT_H
