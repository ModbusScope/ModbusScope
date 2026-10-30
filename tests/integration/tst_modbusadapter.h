#ifndef TST_MODBUSADAPTER_H
#define TST_MODBUSADAPTER_H

#include <QObject>

class AdapterClient;

/*!
 * \brief Integration tests for the shipped modbusadapter binary.
 *
 * Uses the real AdapterProcess and AdapterClient against the modbusadapter
 * executable committed in adapters/Modbus to verify the binary for the build
 * platform loads and speaks the adapter protocol. No Modbus server or license is needed.
 */
class TestModbusAdapter : public QObject
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

#endif // TST_MODBUSADAPTER_H
