/*
 * The internal precision can be set via compilation flags.
 * The number of digits must be coherent with the power.
 * For example, an internal precision for 0.0001 is
 *
 * -DTYPE_DECIMAL_DIGITS=4 -DTYPE_DECIMAL_POWER=10000
 *
 * If not set, the default is
 * -DTYPE_DECIMAL_DIGITS=3 -DTYPE_DECIMAL_POWER=1000
 *
 * Version: v1.0.0
 * Author: Omar Rampado <omar@ognibit.it>
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE    // exp10
#endif

#include "strongtypes.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <err.h>
#include <assert.h>

#ifndef TYPE_DECIMAL_DIGITS
#define TYPE_DECIMAL_DIGITS 3
#define TYPE_DECIMAL_POWER  1000
#endif

static const struct TypeConf *config = NULL;
static int configLen = 0;

static
bool validate_type(int type)
{
    return (type >= 0) && (type < configLen);
}

static
bool validate_range(const TypeValue tv)
{
    return (tv.value >= config[tv.type].rangeMin) &&
           (tv.value <= config[tv.type].rangeMax);
}

#ifndef NDEBUG
static
bool validate_value(const TypeValue tv)
{
    return validate_type(tv.type) && validate_range(tv);
}

static
bool validate_precision(int precision)
{
    return (precision >= 0) && (precision <= TYPE_DECIMAL_DIGITS);
}
#endif

struct TypeConf type_conf_int(type_value_store min, type_value_store max)
{
    struct TypeConf c = {.category=INTEGER,
                         .rangeMin=min,
                         .rangeMax=max,
                         .precision=0};
    return c;
}

struct TypeConf type_conf_dec(type_decimal min, type_decimal max, int precision)
{
    assert(validate_precision(precision));

    struct TypeConf c = {.category=DECIMAL,
                         .rangeMin=min,
                         .rangeMax=max,
                         .precision=precision};
    return c;
}

struct TypeConf type_conf_nom(int count)
{
    assert(count > 0);
    struct TypeConf c = {.category=NOMINAL,
                         .rangeMin=0,
                         .rangeMax=count,
                         .precision=0};
    return c;
}

type_decimal type_dec(double v)
{
    return v * TYPE_DECIMAL_POWER;
}

void type_config(const struct TypeConf *table, int len)
{
    /* sanity checks */
    for (int i=0; i < len; i++){
        assert(table[i].category >= 0);
        assert(table[i].category <= DECIMAL);

        /* can be equals in the corner case of a NOMINAL type with just
         * one possible value */
        assert(table[i].rangeMin <= table[i].rangeMax);

        assert(validate_precision(table[i].precision));
        assert((table[i].category != DECIMAL && table[i].precision == 0) ||
               (table[i].category == DECIMAL && table[i].precision > 0));
    }

    /* set globals */
    config = table;
    configLen = len;
}/* type_config */

TypeValue type_init(int type)
{
    assert(validate_type(type));

    TypeValue t = {.type = type, .value = 0};
#ifdef TYPE_TIMESTAMP
    t.timestamp = 0;
#endif
    return t;
}/* type_init */

int type_type(const TypeValue tv)
{
    assert(validate_type(tv.type));
    return tv.type;
}/* type_type */

type_value_store type_int(const TypeValue tv)
{
    assert(validate_value(tv));
    return tv.value;
}/* type_int */

double type_float(const TypeValue tv)
{
    assert(validate_value(tv));
    return ((double)(tv.value)) / (double)TYPE_DECIMAL_POWER;
}/* type_float */

int type_nom(const TypeValue tv)
{
    assert(validate_value(tv));
    return tv.value;
}/* type_nom */

TypeResult type_seti(const TypeValue tv, type_value_store v)
{
    TypeValue t = {.type = tv.type, .value = v};
#ifdef TYPE_TIMESTAMP
    t.timestamp = type_now();
#endif
    TypeResult res = {.status = TS_OK, .out = t};

    if ((validate_type(tv.type)) &&
        (config[tv.type].category != INTEGER)){
        res.status = TS_INCOMPATIBLE;
        return res;
    }

    if (!validate_range(t)){
        res.status = TS_OUTRANGE;
        return res;
    }

    return res;
}/* type_seti */

TypeResult type_setd(const TypeValue tv, double val)
{
    type_value_store v = type_dec(val);

    /* enforce precision */
    int prec = config[tv.type].precision;
    type_value_store cut = exp10((double)(TYPE_DECIMAL_DIGITS - prec));
    v = (v / cut) * cut; /* integer operations, remove righmost digits */

    TypeValue t = {.type = tv.type, .value = v};
#ifdef TYPE_TIMESTAMP
    t.timestamp = type_now();
#endif
    TypeResult res = {.status = TS_OK, .out = t};

    if ((validate_type(tv.type)) &&
        (config[tv.type].category != DECIMAL)){
        res.status = TS_INCOMPATIBLE;
        return res;
    }


    if (!validate_range(t)){
        res.status = TS_OUTRANGE;
        return res;
    }

    return res;
}/* type_setd */

TypeResult type_setn(const TypeValue tv, int name)
{
    TypeValue t = {.type = tv.type, .value = name};
#ifdef TYPE_TIMESTAMP
    t.timestamp = type_now();
#endif
    TypeResult res = {.status = TS_OK, .out = t};

    if ((validate_type(tv.type)) &&
        (config[tv.type].category != NOMINAL)){
        res.status = TS_INCOMPATIBLE;
        return res;
    }

    if (!validate_range(t)){
        res.status = TS_OUTRANGE;
        return res;
    }

    return res;
}/* type_setn */

/* for internal use only, no input validation needed,
 * valid for INTEGER and DECIMAL since the last one is represented as long too
 */
static
TypeResult value_sum(const TypeValue a, const TypeValue b)
{
    type_value_store va = a.value;
    type_value_store vb = b.value;
    type_value_store sum = 0;
    enum TypeStatus status = TS_OK;

    //sum = va + vb;
    if (__builtin_add_overflow(va, vb, &sum)){
        status = TS_OUTRANGE;
    }

    TypeValue t = {.type = a.type, .value = sum};

    if (!validate_range(t)){
        status = TS_OUTRANGE;
    }

    TypeResult res = {.status = status, .out = t};

    return res;
}/* value_sum */

TypeResult type_sum(const TypeValue a, const TypeValue b)
{
    assert(validate_value(a));
    assert(validate_value(b));

    TypeValue t = {.type = a.type, .value = 0};
    TypeResult res = {.status = TS_INCOMPATIBLE, .out = t};

    if (a.type != b.type){
        return res;
    }

    switch (config[a.type].category){
    case NOMINAL:
        res.status = TS_INCOMPATIBLE;
        break;
    case INTEGER: /* fall through */
    case DECIMAL:
        res = value_sum(a, b);
        break;
    default:
        errx(EXIT_FAILURE, "Invalid category configuration for type: %i", a.type);
        break;
    }/* switch */


#ifdef TYPE_TIMESTAMP
    res.out.timestamp = type_now();
#endif
    return res;
} /* type_sum */

/* for internal use only, no input validation needed */
static
TypeResult integer_mul(const TypeValue a, const TypeValue b)
{
    type_value_store va = a.value;
    type_value_store vb = b.value;
    type_value_store mul = 0;
    enum TypeStatus status = TS_OK;

    //mul = va * vb;
    if (__builtin_mul_overflow(va, vb, &mul)){
        status = TS_OUTRANGE;
    }

    TypeValue t = {.type = a.type, .value = mul};

    if (!validate_range(t)){
        status = TS_OUTRANGE;
    }

    TypeResult res = {.status = status, .out = t};

    return res;
}/* integer_mul */

/* for internal use only, no input validation needed */
static
TypeResult decimal_mul(const TypeValue a, const TypeValue b)
{
    type_value_store va = a.value;
    type_value_store vb = b.value;
    type_value_store mul = 0;
    enum TypeStatus status = TS_OK;

    //mul = va * vb / POWER;
    if (__builtin_mul_overflow(va, vb, &mul)){
        status = TS_OUTRANGE;
    }

    mul = mul / TYPE_DECIMAL_POWER;

    TypeValue t = {.type = a.type, .value = mul};

    if (!validate_range(t)){
        status = TS_OUTRANGE;
    }

    TypeResult res = {.status = status, .out = t};

    return res;
}/* decimal_mul */

TypeResult type_mul(const TypeValue a, const TypeValue b)
{
    assert(validate_value(a));
    assert(validate_value(b));

    TypeValue t = {.type = a.type, .value = 0};
    TypeResult res = {.status = TS_INCOMPATIBLE, .out = t};

    if (a.type != b.type){
        return res;
    }

    switch (config[a.type].category){
    case NOMINAL:
        res.status = TS_INCOMPATIBLE;
        break;
    case INTEGER:
        res = integer_mul(a, b);
        break;
    case DECIMAL:
        res = decimal_mul(a, b);
        break;
    default:
        errx(EXIT_FAILURE, "Invalid category configuration for type: %i", a.type);
        break;
    }/* switch */

#ifdef TYPE_TIMESTAMP
    res.out.timestamp = type_now();
#endif
    return res;
} /* type_mul */

static
TypeResult decimal_div(const TypeValue a, const TypeValue b)
{
    long double va = (long double)a.value;
    long double vb = (long double)b.value;
    type_value_store div = 0;
    enum TypeStatus status = TS_OK;

    div = (va / vb) * TYPE_DECIMAL_POWER;

    /* enforce precision */
    int prec = config[a.type].precision;
    type_value_store cut = exp10((double)(TYPE_DECIMAL_DIGITS - prec));
    div = (div / cut) * cut; /* integer operations, remove righmost digits */

    TypeValue t = {.type = a.type, .value = div};

    if (!validate_range(t)){
        status = TS_OUTRANGE;
    }

    TypeResult res = {.status = status, .out = t};

    return res;
} /* decimal_div */


TypeResult type_div(const TypeValue a, const TypeValue b)
{
    assert(validate_value(a));
    assert(validate_value(b));

    TypeValue t = {.type = a.type, .value = 0};
    TypeResult res = {.status = TS_INCOMPATIBLE, .out = t};

    if (a.type != b.type){
        return res;
    }

    /* avoid division by zero */
    if (config[a.type].category != NOMINAL && b.value == 0) {
        res.status = TS_OUTRANGE;
        return res;
    }

    switch (config[a.type].category){
    case NOMINAL:
        res.status = TS_INCOMPATIBLE;
        break;
    case INTEGER:
        t.value = a.value / b.value;
        res.status = TS_OK;
        res.out = t;
        break;
    case DECIMAL:
        res = decimal_div(a, b);
        break;
    default:
        errx(EXIT_FAILURE, "Invalid category configuration for type: %i", a.type);
        break;
    }/* switch */

#ifdef TYPE_TIMESTAMP
    res.out.timestamp = type_now();
#endif
    return res;
} /* type_div */

int type_dec_units(const TypeValue tv)
{
    assert(config[tv.type].category == DECIMAL);
    return tv.value / TYPE_DECIMAL_POWER;
}/* type_dec_unites */

int type_dec_decimals(const TypeValue tv)
{
    assert(config[tv.type].category == DECIMAL);
    int precision = config[tv.type].precision;
    type_value_store power = exp10((double)(TYPE_DECIMAL_DIGITS - precision));
    return (tv.value % TYPE_DECIMAL_POWER) / power;
}/* type_dec_decimals */

/* Convert DECIMAL to string */
static
void str_decimals(char *buf, const TypeValue tv)
{
    /* format string with decimal precision */
    char fmt[10] = {'\0'};
    sprintf(fmt, "%%i.%%0%ii", config[tv.type].precision);

    snprintf(buf, TYPE_STR_LEN-1, fmt,
                type_dec_units(tv),
                type_dec_decimals(tv));
}/* str_decimals */

void type_str(char *buf, const TypeValue tv)
{
    memset(buf, 0, TYPE_STR_LEN);

    switch (config[tv.type].category){
    case NOMINAL: /* fall through */
    case INTEGER:
        snprintf(buf, TYPE_STR_LEN-1, "%lli", tv.value);
        break;
    case DECIMAL:
        str_decimals(buf, tv);
        break;
    default:
        errx(EXIT_FAILURE, "Invalid category configuration for type: %i", tv.type);
        break;
    }/* switch */
}/* type_str */

#ifdef TYPE_TIMESTAMP
type_millisecs type_get_time(const TypeValue tv)
{
    return tv.timestamp;
}/* type_get_time */
#endif
