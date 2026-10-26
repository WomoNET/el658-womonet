/* Minimal test framework: checks count failures and keep running. */
#ifndef TEST_H
#define TEST_H

#include <stdio.h>

extern int test_checks;
extern int test_failures;
extern const char *test_name;

#define CHECK(cond)                                                                      \
    do {                                                                                 \
        test_checks++;                                                                   \
        if (!(cond)) {                                                                   \
            test_failures++;                                                             \
            printf("%s:%d: %s: CHECK(%s) failed\n", __FILE__, __LINE__, test_name, #cond); \
        }                                                                                \
    } while (0)

#define CHECK_EQ(actual, expected)                                                       \
    do {                                                                                 \
        long actual_ = (long)(actual);                                                   \
        long expected_ = (long)(expected);                                               \
        test_checks++;                                                                   \
        if (actual_ != expected_) {                                                      \
            test_failures++;                                                             \
            printf("%s:%d: %s: %s is %ld, expected %ld\n", __FILE__, __LINE__, test_name, \
                   #actual, actual_, expected_);                                         \
        }                                                                                \
    } while (0)

#define CHECK_BYTES(actual, expected, length)                                            \
    do {                                                                                 \
        for (unsigned i_ = 0; i_ < (length); i_++) {                                     \
            CHECK_EQ((actual)[i_], (expected)[i_]);                                      \
        }                                                                                \
    } while (0)

#define RUN(test)              \
    do {                       \
        test_name = #test;     \
        test();                \
    } while (0)

void test_fade_suite(void);
void test_curve_suite(void);
void test_dimmer_suite(void);
void test_frames_suite(void);
void test_diag_suite(void);
void test_lin_slave_suite(void);
void test_buttons_suite(void);
void test_controller_suite(void);
void test_app_suite(void);

#endif
