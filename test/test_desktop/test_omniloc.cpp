#include "unity.h"

#ifdef UNIT_TEST

// just an example until I figure out what to test
void test_addition(void) {
    TEST_ASSERT_EQUAL(32, 25+7);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_addition);
    UNITY_END();

    return 0;
}

#endif
