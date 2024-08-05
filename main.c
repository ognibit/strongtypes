#include "strongtypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <string.h>

enum PrjTypes {
    HUGE,
    LEVEL,
    POWER,
    COEF,
    STATE,
    KHZ,
    ALL_TYPES /* placeholder */
}; /* PrjTypes */

enum States {
    ON,
    OFF,
    ALL_STATES /* placeholder */
};


static type_millisecs timeMock = 0;

/* Implementation for timestamp management */
type_millisecs type_now(void)
{
    return timeMock;
}

struct TypeConf TYPE_CONFIG[ALL_TYPES];

void init_typeconf()
{
    TYPE_CONFIG[HUGE] = type_conf_int(LONG_MIN, LONG_MAX);
    TYPE_CONFIG[LEVEL] = type_conf_int(-999, 1000);
    TYPE_CONFIG[POWER] = type_conf_int(0, 100);
    TYPE_CONFIG[COEF] = type_conf_dec(type_dec(-3.2), type_dec(3.2), 2);
    TYPE_CONFIG[STATE] = type_conf_nom(ALL_STATES);
    TYPE_CONFIG[KHZ] = type_conf_dec(type_dec(-65536.0), type_dec(65536.0),3);
}

void test_level()
{
    printf("test_level: ");

    TypeResult rc;
    TypeValue lev = type_init(LEVEL);
    assert(type_type(lev) == LEVEL);

    rc = type_setd(lev, 3.0);
    assert(rc.status == TS_INCOMPATIBLE);

    rc = type_seti(lev, -1000);
    assert(rc.status == TS_OUTRANGE);

    rc = type_seti(lev, 1001);
    assert(rc.status == TS_OUTRANGE);

    rc = type_seti(lev, 1000);
    assert(rc.status == TS_OK);
    lev = rc.out;

    TypeValue l1 = type_init(LEVEL);
    rc = type_seti(l1, -999);
    assert(rc.status == TS_OK);
    l1 = rc.out;

    rc = type_sum(lev, l1);
    assert(rc.status == TS_OK);
    assert(type_int(rc.out) == 1);

    rc = type_mul(lev, l1);
    assert(rc.status == TS_OUTRANGE);

    rc = type_seti(lev, -1);
    assert(rc.status == TS_OK);
    lev = rc.out;

    rc = type_mul(lev, l1); /* -1 * -999 */
    assert(rc.status == TS_OK);
    assert(type_int(rc.out) == 999);

    /* DIVISION */
    TypeValue a = type_init(LEVEL);
    TypeValue b = type_init(LEVEL);
    a = type_seti(a, -124).out;
    b = type_seti(b, -2).out;

    rc = type_div(a, b);
    assert(rc.status == TS_OK);
    assert(type_int(rc.out) == 62);

    rc = type_div(b, a);
    assert(rc.status == TS_OK);
    assert(type_int(rc.out) == 0);

    printf("OK\n");
}/* test_level */


void test_state()
{
    printf("test_state: ");

    TypeResult rc;
    TypeValue st_on = type_init(STATE);
    assert(type_type(st_on) == STATE);

    rc = type_setd(st_on, 3.0);
    assert(rc.status == TS_INCOMPATIBLE);

    rc = type_seti(st_on, -1000);
    assert(rc.status == TS_INCOMPATIBLE);

    rc = type_setn(st_on, 10);
    assert(rc.status == TS_OUTRANGE);

    rc = type_setn(st_on, ON);
    assert(rc.status == TS_OK);
    assert(type_nom(st_on) == ON);
    st_on = rc.out;
    assert(type_type(st_on) == STATE);

    rc = type_sum(st_on, st_on);
    assert(rc.status == TS_INCOMPATIBLE);

    rc = type_mul(st_on, st_on);
    assert(rc.status == TS_INCOMPATIBLE);

    rc = type_div(st_on, st_on);
    assert(rc.status == TS_INCOMPATIBLE);

    printf("OK\n");
}/* test_state */

void test_coef()
{
    printf("test_coef: ");

    TypeResult rc;
    TypeValue coef = type_init(COEF);
    TypeValue one = type_init(COEF);
    assert(type_type(one) == COEF);

    rc = type_seti(coef, -1000);
    assert(rc.status == TS_INCOMPATIBLE);

    rc = type_setn(coef, ON);
    assert(rc.status == TS_INCOMPATIBLE);

    rc = type_setd(coef, 3.1477);
    assert(rc.status == TS_OK);
    assert(type_float(rc.out) == 3.140);

    rc = type_setd(coef, 3.14);
    assert(rc.status == TS_OK);
    coef = rc.out;

    rc = type_setd(one, -1.11);
    assert(rc.status == TS_OK);
    one = rc.out;
    assert(type_type(one) == COEF);

    rc = type_sum(coef, one);
    assert(rc.status == TS_OK);
    assert(type_float(rc.out) == 2.03);

    rc = type_mul(coef, one);
    assert(rc.status == TS_OUTRANGE);

    /* negative */
    rc = type_setd(one, -0.9);
    assert(rc.status == TS_OK);
    assert(type_float(rc.out) == -0.9);
    one = rc.out;

    /* 3.14 * -0.9 =  -2.826 */
    rc = type_mul(coef, one);
    assert(rc.status == TS_OK);
    assert(type_float(rc.out) < -2.81 && type_float(rc.out) > -2.83);

    printf("OK\n");
}/* test_coef */

void test_overflow()
{
    printf("test_overflow: ");

    TypeResult rc;
    TypeValue ai = type_init(HUGE);
    TypeValue bi = type_init(HUGE);

    //overflow
    ai = type_seti(ai, LONG_MAX - 50).out;
    bi = type_seti(bi, 60).out;
    rc = type_sum(ai, bi);
    assert(rc.status == TS_OUTRANGE);

    //underflow
    ai = type_seti(ai, -1).out;
    bi = type_seti(bi, LONG_MIN).out;
    rc = type_sum(ai, bi);
    assert(rc.status == TS_OUTRANGE);

    // MULTIPLICATION
    ai = type_seti(ai, 12345654321).out;
    bi = type_seti(bi, 65432123456).out;
    rc = type_mul(ai, bi);
    assert(rc.status == TS_OUTRANGE);

    printf("OK\n");
}/* test_overflow */

void test_khz()
{
    printf("test_khz: ");

    TypeResult rc;
    TypeValue a = type_init(KHZ);
    TypeValue b = type_init(KHZ);

    a = type_setd(a, 6.8).out;
    b = type_setd(b, -3.2).out;

    rc = type_div(a, b);
    assert(rc.status == TS_OK);
    assert(type_float(rc.out) == -2.125);

    rc = type_div(b, a); /* -3.2 / 6.8 = -0470588... */
    assert(rc.status == TS_OK); /* precision = 3 */
    assert(type_float(rc.out) >= -0.470  && type_float(rc.out) < -0.469);

    printf("OK\n");
}/* test_khz */

void test_str()
{
    printf("test_str: ");
    char buf[TYPE_STR_LEN];

    TypeValue in = type_init(HUGE);
    TypeValue no = type_init(STATE);
    TypeValue de = type_init(KHZ);

    in = type_seti(in, 1234567890).out;
    no = type_setn(no, OFF).out;
    de = type_setd(de, 61234.32).out;

    type_str(buf, in);
    assert(strncmp("1234567890", buf, TYPE_STR_LEN) == 0);

    type_str(buf, no);
    assert(strncmp("1", buf, TYPE_STR_LEN) == 0);

    type_str(buf, de); /* precision = 3 */
    assert(strncmp("61234.320", buf, TYPE_STR_LEN) == 0);

    de = type_setd(de, 61234.0).out;
    type_str(buf, de); /* precision = 3 */
    assert(strncmp("61234.000", buf, TYPE_STR_LEN) == 0);

    printf("OK\n");
}

void test_timestamp(void)
{
    char buf[TYPE_STR_LEN];

    printf("test_timestamp: ");

    timeMock = 100;

    TypeValue in = type_init(HUGE);
    TypeValue no = type_init(STATE);
    TypeValue de = type_init(KHZ);

    assert(type_get_time(in) == 0);
    assert(type_get_time(no) == 0);
    assert(type_get_time(de) == 0);

    in = type_seti(in, 1234567890).out;
    no = type_setn(no, OFF).out;
    de = type_setd(de, 61234.32).out;

    assert(type_get_time(in) == 100);
    assert(type_get_time(no) == 100);
    assert(type_get_time(de) == 100);

    /* test readonly operation, the time is NOT updated */
    timeMock = 200;

    type_type(in);
    type_type(no);
    type_type(de);

    assert(type_get_time(in) == 100);
    assert(type_get_time(no) == 100);
    assert(type_get_time(de) == 100);

    type_int(in);
    type_nom(no);
    type_float(de);

    assert(type_get_time(in) == 100);
    assert(type_get_time(no) == 100);
    assert(type_get_time(de) == 100);

    type_str(buf, in);
    type_str(buf, no);
    type_str(buf, de);

    assert(type_get_time(in) == 100);
    assert(type_get_time(no) == 100);
    assert(type_get_time(de) == 100);

    /* math operation, must set the timestamp on the return */
    timeMock = 300;

    assert(type_get_time(type_sum(in, in).out) == 300);
    assert(type_get_time(type_sum(de, de).out) == 300);

    assert(type_get_time(type_mul(in, in).out) == 300);
    assert(type_get_time(type_mul(de, de).out) == 300);

    assert(type_get_time(type_div(in, in).out) == 300);
    assert(type_get_time(type_div(de, de).out) == 300);

    /* the input must be untouched */
    assert(type_get_time(in) == 100);
    assert(type_get_time(no) == 100);
    assert(type_get_time(de) == 100);

    printf("OK\n");
}


int main()
{
    init_typeconf();
    type_config(TYPE_CONFIG, ALL_TYPES);

    test_level();
    test_state();
    test_coef();
    test_overflow();
    test_khz();
    test_str();
    test_timestamp();

    return 0;
}
