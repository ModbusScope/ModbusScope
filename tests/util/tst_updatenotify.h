
#include <QObject>

#include "updatenotify.h"

/* Forward declaration */
class MockVersionDownloader;

class TestUpdateNotify: public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void triggerDownloader();

    void versionLowerRevision();
    void versionLowerMinor();
    void versionLowerMajor();
    void versionEqual();

    void versionHigherRevision();
    void versionHigherMinor();
    void versionHigherMajor();

    void incorrectVersion();
    void incorrectUrl();

private:

    void configureServerData(QString version, QString url, QString publishDate);
    void checkServerCheck(QString version, UpdateNotify::UpdateState updateState);

    MockVersionDownloader* _pVersionDownloader;

    QString _invalidPublishData;
    QString _tooRecentPublishData;
    QString _notRecentPublishData;

    static const QString _cVersion;

    static const QString _cVersionLowerRevision;
    static const QString _cVersionLowerMinor;
    static const QString _cVersionLowerMajor;

    static const QString _cVersionHigherRevision;
    static const QString _cVersionHigherMinor;
    static const QString _cVersionHigherMajor;

};
