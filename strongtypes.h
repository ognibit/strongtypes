#pragma once

/*
 * Strong types.
 * A module for enforcing range and type control on numeric data.
 *
 * Version: v1.0.0
 * Author: Omar Rampado <omar@ognibit.it>
 */

#define TYPE_STR_LEN    24

typedef long long ValueStore;

/* General typed value container */
struct TypeValue {
    int type;
    ValueStore value;
};

enum TypeCategory {
    NOMINAL, /* can only be compared */
    INTEGER,
    DECIMAL
};

enum TypeStatus {
    TS_OK,
    TS_OUTRANGE,
    TS_INCOMPATIBLE
};

/* Configuration for a type */
struct TypeConf {
    enum TypeCategory category;
    ValueStore range_min;
    ValueStore range_max;
    int precision; /* 0-6 */
};

/* Operation result on types */
struct TypeResult {
    enum TypeStatus status;
    struct TypeValue out;
};

typedef struct TypeValue TypeValue;
typedef struct TypeResult TypeResult;
typedef ValueStore Decimal;

/* create a decimal value, for range set, from a floating point */
Decimal type_dec(double v);

/* Create the configuration for an INTEGER type */
struct TypeConf type_conf_int(ValueStore min, ValueStore max);

/* Create the configuration for a DECIMAL type */
struct TypeConf type_conf_dec(Decimal min, Decimal max, int precision);

/* Create the configuration for a NOMINAL type.
 * count: the number of the possible categorical values.
 */
struct TypeConf type_conf_nom(int count);

/* Mandatory first operation.
 * The table memory must be always available and immutable.
 */
void type_config(const struct TypeConf *table, int len);

/* init a new type instance at 0.
 * After init, use the corresponding set operation.
 * It should be used only with constants because
 * it validates the type and abort in case of error.
 */
TypeValue type_init(int type);

/* get the type.
 * It should be used only with constants because
 * it validates the type and abort in case of error.
 */
int type_type(const TypeValue tv);

/* get the integer value.  */
ValueStore type_int(const TypeValue tv);

/* get an approximation of the decimal value */
double type_float(const TypeValue tv);

/* get the nominal value. */
int type_nom(const TypeValue tv);

/* create a copy with the integer value set.  */
TypeResult type_seti(const TypeValue tv, ValueStore v);

/* Set a decimal value, will be truncated to precision. */
TypeResult type_setd(const TypeValue tv, double v);

/* Set a nominal value.
 * E.g. type_setn(v, STATE_A);
 * where STATE_A is an enumeration value.
 */
TypeResult type_setn(const TypeValue tv, int name);

/* sum */
TypeResult type_sum(const TypeValue a, const TypeValue b);

/* multiplication */
TypeResult type_mul(const TypeValue a, const TypeValue b);

/* division */
TypeResult type_div(const TypeValue a, const TypeValue b);

/* get the representation of the value in string format.
 * The nominal values are just the integer represenration.
 * The decimal values show the precision digits, pad with zeros.
 * The buf must be at least TYPE_STR_LEN long.
 * No validation checks applied.
 */
void type_str(char *buf, const TypeValue tv);
