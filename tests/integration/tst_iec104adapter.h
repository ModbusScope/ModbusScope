#ifndef TST_IEC104ADAPTER_H
#define TST_IEC104ADAPTER_H

#include <QObject>

class AdapterClient;

/*!
 * \brief Integration tests for the shipped iec104adapter binary.
 *
 * Uses the real AdapterProcess and AdapterClient against the iec104adapter
 * executable committed in adapters/IEC104 to verify the binary for the build
 * platform loads and speaks the adapter protocol. No IEC 104 server is needed.
 */
class TestIec104Adapter : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void cleanup();
    void describeResultHasRequiredFields();
    void fullLifecycleSessionStarts();
    void stopSessionEmitsSessionStopped();
    void dataPointSchemaReturnsRequiredFields();
    void describeDataPointReturnsStructuredFields();
    void validateDataPointReturnsTrueForValidExpression();
    void validateDataPointReturnsFalseForInvalidExpression();

private:
    AdapterClient* _pClient{ nullptr };
};

#endif // TST_IEC104ADAPTER_H
