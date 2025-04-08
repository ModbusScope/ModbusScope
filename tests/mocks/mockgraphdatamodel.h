#ifndef MOCKGRAPHDATAMODEL_H__
#define MOCKGRAPHDATAMODEL_H__

#include "models/graphdatamodel.h"
#include "models/settingsmodel.h"

#include "gmock/gmock.h"

class MockGraphDataModel : public GraphDataModel {
    public:
        MockGraphDataModel() : GraphDataModel(new SettingsModel()) {}

        MOCK_METHOD2(isPresent, bool(quint32 addr, QString* pExpression));
};

#endif /* MOCKGRAPHDATAMODEL_H__ */
