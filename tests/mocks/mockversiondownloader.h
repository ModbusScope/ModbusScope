#ifndef MOCKVERSIONDOWNLOADER_H__
#define MOCKVERSIONDOWNLOADER_H__

#include "util/versiondownloader.h"

#include "gmock/gmock.h"

class MockVersionDownloader : public VersionDownloader {
    public:

        MOCK_METHOD(void, performCheck, ());
        MOCK_METHOD(QString, version, ());
        MOCK_METHOD(QString, url, ());
        MOCK_METHOD(QString, publishDate, ());

};

#endif /* MOCKVERSIONDOWNLOADER_H__ */
