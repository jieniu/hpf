#include "../base_define.h"
#include <assert.h>

int test_ret_if_fail = 0;

void test_ret_if_fail_func()
{
    ret_if_fail(1==2);
    test_ret_if_fail = 1;
}

void test_ret_if_succ()
{
    ret_if_fail(2==2);
    test_ret_if_fail = 1;
}

int test_ret_value_if_fail()
{
    ret_val_if_fail(1==2, -1);

    return 0;
}

int test_ret_value_if_succ()
{
    ret_val_if_fail(2==2, -1);

    return 0;
}

int main()
{
    test_ret_if_fail_func();
    assert(test_ret_if_fail == 0);
    test_ret_if_succ();
    assert(test_ret_if_fail == 1);

    assert(-1 == test_ret_value_if_fail());
    assert(0 == test_ret_value_if_succ());

    return 0;
}
