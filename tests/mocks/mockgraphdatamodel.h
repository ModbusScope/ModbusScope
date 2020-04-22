#ifndef MOCKGRAPHDATAMODEL_H__
#define MOCKGRAPHDATAMODEL_H__

#include "gmock/gmock.h"

#include "graphdatamodel.h"
#include "settingsmodel.h"

class MockGraphDataModel : public GraphDataModel {
    public:
        MockGraphDataModel() : GraphDataModel(new SettingsModel()) {}

        MOCK_METHOD2(isPresent, bool(quint16 addr, quint32 bitmask));
};

#endif /* MOCKGRAPHDATAMODEL_H__ */
