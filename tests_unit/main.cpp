#include "tst_errorlog.h"
#include "tst_errorlogmodel.h"
#include "tst_mbcregistermodel.h"
#include "tst_readregisters.h"
#include "tst_graphdata.h"

#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
