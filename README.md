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
