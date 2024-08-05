# Strong Types for C

A simple library of functions to manage ranges and precision for numeric values
in C/C++. The library is primarily written for C.

The source code should be directly included in the project codebase and used
for the numbers in the application that need a pedantic verification of
boundaries. Since there is a significant overhead, both on memory and
performance, it should be used only where safety is involved.

The library functions do assertions at every call that can be disabled using
the normal `-DNDEBUG` compilation flag.
It is better to develop the project with the assertions enabled and remove them
only for production code, to avoid crashes.
Other validations are still in place for the functions designed to operate with
external values, such as the sum, multiplication, and division functions.
The function caller must manage the result status and explicitly assign the
result value (`out`) to the variable if no errors are present.

The addition (`type_sum`) and the multiplication (`type_mul`) do check also the
overflow and underflow of the operation.

See the `main.c` file for the usage.

## Internal Precision

Compilation flag: `TYPE_DECIMAL_DIGITS`, `TYPE_DECIMAL_POWER`

The decimal precision is not arbitrary. The maximum number of decimal digits
(after the dot) must be set according with the corresponding power of ten.
These values set the space dedicated to the decimal precision and reduce the
space for the unit part.

If not set, the default is:

- `TYPE_DECIMAL_DIGITS = 3`
- `TYPE_DECIMAL_POWER  = 1000`

## Timestamp

Compilation flag: `TYPE_TIMESTAMP`.

With the compilation flag set (default is disable) every value carries also
a timestamp with it. The time correspond to the last operation done on the
value. Setting a value to the same number still changes the timestamp.

This feature can be used to keep track of the refresh rate of a value and to
understand if a value is still valid.

The user **must** provide an implementation for

```
type_millisecs type_now(void);
```

The function must not fail and it must be reliable.
The function should increase monotonically.
If an overflow happens, it must be managed by the application.
The module does not use the value in the internal logic.

The feature enable the function for retrieving the timestamp

```
type_millisecs type_get_time(const TypeValue);
```

## TODO

- Use just `TYPE_DECIMAL_POWER`
