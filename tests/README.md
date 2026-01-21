## Tests

I wrote tests for the lexer, the parser, the environments and the semantic analyzer. I try all the book testcases and I also wrote a couple of my own, to cover many more possibilities.

Moreover, inside `misc` there are other examples, such as the prime sieve. Here, there's a runner script written in Bash that picks up all the `.tig` files in the same folder, compiles them using the path to an installed compiler (driver) that you must provide after running `make install`, and compares the program output to a predefined value. The outputs must match for each test to pass. 