#ifndef MOCKADAPTERHUB_H
#define MOCKADAPTERHUB_H

#include "ProtocolAdapter/adapterhub.h"
#include "ProtocolAdapter/adaptermanager.h"

#include <QMap>

/*!
 * \brief Test double for AdapterHub.
 *
 * Holds externally created (mock) adapter managers and serves them via the
 * regular lookup interface without starting any adapter infrastructure.
 */
class MockAdapterHub : public AdapterHub
{
public:
    explicit MockAdapterHub(QObject* parent = nullptr) : AdapterHub(parent)
    {
    }

    void addManager(const QString& id, AdapterManager* pManager)
    {
        _managers.insert(id, pManager);
    }

    AdapterManager* adapterManager(const QString& id) const override
    {
        return _managers.value(id, nullptr);
    }

    QStringList adapterIds() const override
    {
        return _managers.keys();
    }

private:
    QMap<QString, AdapterManager*> _managers;
};

#endif // MOCKADAPTERHUB_H
