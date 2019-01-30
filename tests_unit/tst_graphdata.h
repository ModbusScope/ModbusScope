#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "src/models/graphdata.h"

#include "src/util/util.h"

using namespace testing;

/* TODO: Mock Util.h */

TEST(GraphData, setLabel)
{
    GraphData graphData;

    QString separatorString = QString(Util::separatorCharacter());
    QString baseString = QString("TEST");

    graphData.setLabel(baseString);
    EXPECT_EQ(graphData.label(), baseString);

    QString testString1 = baseString + separatorString;
    graphData.setLabel(testString1);
    EXPECT_EQ(graphData.label(), baseString);

    QString testString2 = separatorString + baseString;
    graphData.setLabel(testString2);
    EXPECT_EQ(graphData.label(), baseString);

    QString testString3 = separatorString + baseString + separatorString;
    graphData.setLabel(testString3);
    EXPECT_EQ(graphData.label(), baseString);

    QString testString4 = baseString + separatorString + baseString;
    graphData.setLabel(testString4);
    EXPECT_EQ(graphData.label(), baseString + baseString);
}


/* TODO: Add extra test for other functions */
