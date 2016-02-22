
#ifdef GTEST_MODE
#include <gtest/gtest.h>
#include "../../util/String.h"
TEST(StringTest, aaaa) {
    String s;
    ASSERT_EQ(0, s.length());
}

#endif