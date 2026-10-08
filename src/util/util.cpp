
#include "util.h"

#include <QDir>
#include <QFileInfo>

/* For Markdown rendering
<span style="display:inline-block; width:20px; height:20px; background: rgb(0, 0, 0);"></span>
<span style="display:inline-block; width:20px; height:20px; background: rgb(0, 0, 255);"></span>
<span style="display:inline-block; width:20px; height:20px; background: rgb(0, 255, 255);"></span>
<span style="display:inline-block; width:20px; height:20px; background: rgb(0, 255, 0);"></span>
<span style="display:inline-block; width:20px; height:20px; background: rgba(255, 40, 40, 1);"></span>
<span style="display:inline-block; width:20px; height:20px; background: rgb(128, 0, 128);"></span>
<span style="display:inline-block; width:20px; height:20px; background: rgba(112, 49, 3, 1);"></span>
<span style="display:inline-block; width:20px; height:20px; background: rgba(250, 2, 134, 1);"></span>
<span style="display:inline-block; width:20px; height:20px; background: rgba(45, 126, 184, 1);"></span>
<span style="display:inline-block; width:20px; height:20px; background: rgba(0, 170, 74, 1);"></span>
<span style="display:inline-block; width:20px; height:20px; background: rgb(220, 220, 0);"></span>
<span style="display:inline-block; width:20px; height:20px; background: rgb(255, 165, 0);"></span>
*/

// clang-format off
const QList<QColor> Util::cColorlist = QList<QColor>() << QColor(0, 0, 0)
                                                       << QColor(0, 0, 255)
                                                       << QColor(0, 255, 255)
                                                       << QColor(0, 255, 0)
                                                       << QColor(255, 40, 40)
                                                       << QColor(128, 0, 128)
                                                       << QColor(112, 49, 3)
                                                       << QColor(250, 2, 134)
                                                       << QColor(45, 126, 184)
                                                       << QColor(0, 170, 74)
                                                       << QColor(220, 220, 0)
                                                       << QColor(255, 165, 0);
// clang-format on
//! \brief Find the bundled demo data file.
//! \param appDir Directory of the application executable.
//! \return Path of the demo file, or an empty string if it is not present.
QString Util::findDemoFile(const QString& appDir)
{
    const QStringList candidates = { QDir(appDir).filePath("demo.csv"),
                                     QDir(appDir).filePath("../share/modbusscope/demo.csv") };

    for (const QString& candidate : candidates)
    {
        if (QFileInfo::exists(candidate))
        {
            return QFileInfo(candidate).canonicalFilePath();
        }
    }

    return QString();
}
