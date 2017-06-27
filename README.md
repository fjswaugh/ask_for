Ask_for
=======

Ask_for is a simple C++ 14 header library (really only one function), that
lets you ask a command-line user for input, without the fuss of verifying it
manually. Ask for an integer, the function will not return until the user
enters one.

Example use
-----------

The following is a program that asks for an integer divisible by two, and
stores the result in the variable `x`. If the user enters an odd number, or a
character that is not a digit, the function will carry on asking indefinitely.
The function works by reading input line by line as its entered, then
attempting to parse the contents of the line.

```cpp
#include "ask_for.h"

int main()
{
    auto x = ask_for<int>("Enter an even integer: ",
                          [](auto i) { return i % 2 == 0; },
                          "Number must be even");
}
```

This program asks for a list of numbers (the header file overloads `operator>>`
on `std::vector` and `std::array` for convenience).

```cpp
#include "ask_for.h"

int main()
{
    auto x = ask_for<std::vector<int>>();
}
```

You can also ask for multiple objects. Say you wanted an int and a double, and
both had to be greater than 100:

```cpp
#include "ask_for.h"

int main()
{
    auto x = ask_for<int, double>("Enter an int then a double: ",
                                  [](auto n) { return n > 100; });
}
```

The type returned in this case is a `std::tuple<int, double>`.

Note
----

This library is not designed to be robust, fast, or particularly flexible. It
is simply a convenience function for myself.
