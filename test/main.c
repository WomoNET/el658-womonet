#include "test.h"

int test_checks;
int test_failures;
const char *test_name;

int main(void)
{
    test_fade_suite();
    test_curve_suite();
    test_dimmer_suite();
    test_frames_suite();
    test_diag_suite();
    test_lin_slave_suite();
    test_buttons_suite();
    test_controller_suite();
    test_app_suite();

    printf("%d checks, %d failed\n", test_checks, test_failures);
    return test_failures == 0 ? 0 : 1;
}
