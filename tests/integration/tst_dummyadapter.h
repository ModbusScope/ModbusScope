#ifndef TST_DUMMYADAPTER_H
#define TST_DUMMYADAPTER_H

#include <QObject>

class AdapterClient;

/*!
 * \brief Integration tests for the dummymodbusadapter binary.
 *
 * Uses the real AdapterProcess and AdapterClient against the dummymodbusadapter
 * executable to verify the full adapter lifecycle end-to-end.
 */
class TestDummyAdapter : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void describeResultHasRequiredFields();
    void fullLifecycleSessionStarts();
    void readRegistersReturnsValidData();
    void multipleReadCyclesAllSucceed();
    void stopSessionEmitsSessionStopped();
    void dataPointSchemaReturnsRequiredFields();
    void describeDataPointReturnsStructuredFields();
    void validateDataPointReturnsTrueForValidExpression();
    void validateDataPointReturnsFalseForInvalidExpression();

private:
    AdapterClient* _pClient{ nullptr };
};

#endif // TST_DUMMYADAPTER_H
