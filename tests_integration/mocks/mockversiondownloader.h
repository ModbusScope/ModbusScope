
#include "gmock/gmock.h"

#include "versiondownloader.h"

class MockVersionDownloader : public VersionDownloader {
    public:

        MOCK_METHOD(void, performCheck, ());
        MOCK_METHOD(QString, version, ());
        MOCK_METHOD(QString, url, ());

};

