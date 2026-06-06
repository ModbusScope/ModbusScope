
#include "ProtocolAdapter/adapterdiscovery.h"

#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>

/*!
 * \brief Scan \a appDir for adapter binaries and return one AdapterInfo per found binary.
 *
 * A file whose base name (without extension) ends with \c "adapter" is treated as an
 * adapter binary. The adapter ID is the prefix before that suffix.
 * E.g. \c modbusadapter (Linux) or \c modbusadapter.exe (Windows) → id \c "modbus".
 *
 * \param appDir Absolute path to the directory to scan (typically the application dir).
 * \return List of AdapterInfo structs, one per discovered adapter. Order is unspecified.
 */
QList<AdapterInfo> AdapterDiscovery::discover(const QString& appDir)
{
    static const QString cAdapterSuffix = QStringLiteral("adapter");

    QList<AdapterInfo> result;
    const QFileInfoList entries = QDir(appDir).entryInfoList(QDir::Files);

    for (const QFileInfo& fi : entries)
    {
        const QString baseName = fi.completeBaseName();
        if (!baseName.endsWith(cAdapterSuffix, Qt::CaseInsensitive))
        {
            continue;
        }
#ifdef Q_OS_WIN
        if (fi.suffix().compare(QStringLiteral("exe"), Qt::CaseInsensitive) != 0)
        {
            continue;
        }
#else
        if (!fi.isExecutable())
        {
            continue;
        }
#endif
        const QString id = baseName.chopped(cAdapterSuffix.length());
        if (!id.isEmpty())
        {
            result.append({ id, fi.absoluteFilePath() });
        }
    }

    return result;
}
