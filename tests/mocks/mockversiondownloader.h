#ifndef MOCKVERSIONDOWNLOADER_H__
#define MOCKVERSIONDOWNLOADER_H__

#include "gmock/gmock.h"

#include "versiondownloader.h"

class MockVersionDownloader : public VersionDownloader {
    public:

        MOCK_METHOD(void, performCheck, ());
        MOCK_METHOD(QString, version, ());
        MOCK_METHOD(QString, url, ());

};

#endif /* MOCKVERSIONDOWNLOADER_H__ */
