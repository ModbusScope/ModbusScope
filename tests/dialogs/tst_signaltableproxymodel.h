#ifndef TST_SIGNALTABLEPROXYMODEL_H
#define TST_SIGNALTABLEPROXYMODEL_H

#include "ProtocolAdapter/adaptermanager.h"

#include <QJsonObject>
#include <QObject>

class DataPointDescriptionCache;
class DataPointService;
class GraphDataModel;
class MockAdapterHub;
class SettingsModel;
class SignalTableProxyModel;

/*!
 * \brief Test double for AdapterManager that captures describeDataPoint() calls and answers them
 * on request.
 */
class MockDescribeManager : public AdapterManager
{
    Q_OBJECT

public:
    explicit MockDescribeManager(QObject* parent = nullptr)
        : AdapterManager(QStringLiteral("modbus"), QString(), nullptr, parent)
    {
    }

    int describeDataPoint(const QString& address) override
    {
        describeCalls.append(address);
        return static_cast<int>(describeCalls.size());
    }

    bool isAdapterReady() const override
    {
        return true;
    }

    //! Simulate the adapter describing the data point of a request (1 for the first call).
    void injectDescribeResult(int requestId, const QString& description)
    {
        QJsonObject result;
        result["valid"] = true;
        result["description"] = description;
        emit describeDataPointResult(requestId, result);
    }

    //! Simulate the adapter rejecting the data point of a request.
    void injectDescribeInvalid(int requestId, const QString& error)
    {
        QJsonObject result;
        result["valid"] = false;
        result["error"] = error;
        emit describeDataPointResult(requestId, result);
    }

    QStringList describeCalls;
};

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
    MockDescribeManager* _pManager{ nullptr };
    MockAdapterHub* _pHub{ nullptr };
    DataPointService* _pService{ nullptr };
    DataPointDescriptionCache* _pCache{ nullptr };
    SignalTableProxyModel* _pProxy{ nullptr };
};

#endif // TST_SIGNALTABLEPROXYMODEL_H
