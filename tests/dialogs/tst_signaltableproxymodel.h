#ifndef TST_SIGNALTABLEPROXYMODEL_H
#define TST_SIGNALTABLEPROXYMODEL_H

#include <QObject>

class DataPointDescriptionCache;
class DataPointService;
class GraphDataModel;
class MockAdapterHub;
class MockDataPointManager;
class SettingsModel;
class SignalTableProxyModel;

class TestSignalTableProxyModel : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void modelTester();
    void extraColumnAfterSourceColumns();
    void extraColumnIsReadOnlyAndHasNoSource();
    void rowSelectionMapsToSourceColumnsOnly();

    void pendingShowsReference();
    void describedShowsDescription();
    void descriptionUpdatesOnlyAffectedRow();
    void invalidShowsReferenceInRedWithError();
    void invalidReferenceNamedInExpressionToolTip();
    void severalReferencesShowCount();
    void duplicateReferencesListedTwice();
    void syntaxErrorShowsParserError();
    void unknownDeviceShownInRed();
    void richTextErrorShownAsPlainText();
    void oneLookupPerReference();
    void noReferenceIsEmpty();
    void expressionChangeUpdatesExtraColumn();

    void mimeDataFromExtraColumnKeepsRow();

private:
    QString text(int row, int role = Qt::DisplayRole) const;

    GraphDataModel* _pGraphDataModel{ nullptr };
    SettingsModel* _pSettingsModel{ nullptr };
    MockDataPointManager* _pManager{ nullptr };
    MockAdapterHub* _pHub{ nullptr };
    DataPointService* _pService{ nullptr };
    DataPointDescriptionCache* _pCache{ nullptr };
    SignalTableProxyModel* _pProxy{ nullptr };
};

#endif // TST_SIGNALTABLEPROXYMODEL_H
