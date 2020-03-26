
#include "gmock/gmock.h"

#include "../src/models/graphdatamodel.h"
#include "../src/models/settingsmodel.h"

class MockGraphDataModel : public GraphDataModel {
    public:
        MockGraphDataModel() : GraphDataModel(new SettingsModel()) {}

        MOCK_METHOD2(isPresent, bool(quint16 addr, quint32 bitmask));
};

