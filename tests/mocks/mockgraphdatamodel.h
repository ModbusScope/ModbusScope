#ifndef MOCKGRAPHDATAMODEL_H__
#define MOCKGRAPHDATAMODEL_H__

#include "gmock/gmock.h"

#include "graphdatamodel.h"
#include "settingsmodel.h"

class MockGraphDataModel : public GraphDataModel {
    public:
        MockGraphDataModel() : GraphDataModel(new SettingsModel()) {}

        MOCK_METHOD2(isPresent, bool(quint32 addr, QString* pExpression));
};

#endif /* MOCKGRAPHDATAMODEL_H__ */
