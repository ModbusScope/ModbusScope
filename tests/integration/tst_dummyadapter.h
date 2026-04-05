#ifndef TST_DUMMYADAPTER_H
#define TST_DUMMYADAPTER_H

#include <QObject>

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
    void describeReturnsRequiredFields();
    void describeNameIsModbusAdapter();
    void fullLifecycleSessionStarts();
    void readRegistersReturnsValidData();
    void multipleReadCyclesAllSucceed();
    void stopSessionEmitsSessionStopped();
};

#endif // TST_DUMMYADAPTER_H
