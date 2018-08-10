
#include "gmock/gmock.h"

#include "src/models/graphdatamodel.h"

class MockGraphDataModel : public GraphDataModel {
 public:
  MOCK_METHOD2(isPresent,
      bool(quint16 addr, quint16 bitmask));
};

