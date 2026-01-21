# Table of Contents
1. [The Tiger language (vanilla)](#tiger)
    1. [Lexical issues](#lex)
    2. [Declarations](#dec)
    3. [Expressions](#exp)
    4. [Scope rules](#scope)
    5. [Programs](#prog)
    6. [Standard Library](#sl)
2. [Requirements](#req)
3. [Build instructions](#build)
4. [Compilation](#compile)
5. [TODO](#todo)

<a id="tiger"></a>
# 1. The Tiger language (vanilla)

The Tiger language is a small language with nested functions, record values with implicit pointers, arrays, integer and string variables and a few simple structured control constructs.

The predefined function `getchar` has been renamed to `getchr` to avoid link problems with the C function. Not all escape sequences are supported. See 1.3.4.

<a id="lex"></a>
## 1.1 Lexical issues

An **identifier** is a sequence of letters, digits and underscores, starting with a letter. Identifiers are case sensitive.
<br>
A **comment** may appear between any two tokens. Comments start with `/*` and end with `*/` and may be nested.
<br>
In the following `ε` denotes an empty string, while `{x}` stands for a possibily empty sequence of `x`'s. Keywords or characters that stand for themselves are written between quotes `''`.
<br>
Words containing the term 'id' are identifiers.

<a id="dec"></a>
## 1.2 Declarations
A *declaration-sequence* is a sequence of type, value, and function declarations;
no punctuation separates or terminates individual declarations.

``` 
decs -> {dec}

dec -> tydec
    -> vardec
    -> fundec
```

### 1.2.1 Type declarations

Tiger has two built-in types: `int` and `string`.
<br>
Additional named types may be defined or redefined by type declarations.
The following syntax is used to create types:

```
tydec -> 'type' type-id '=' ty

ty -> type-id
   -> '{' tyfields '}'
   -> 'array' 'of' type-id

tyfields -> ε
         -> id ':' type-id {',' id ':' type-id}
```

**Records** are defined by listing of their fields enclosed in braces, with each field described by `fieldname: type-id`, where `type-id` is an identifier defined by a type declaration or a built-in type. Records can contain no field.
<br>
<br>
**Arrays** are defined using the syntax `array of type-id`. The length of the array is not specified as part of its type; each array of that type can have a different length, and the length will be decided upon array creation, at run time.
<br>
<br>
Each declaration of a record or array type creates a new type, incompatible with all other record or array types (even if all fields are similar).
A collection of types may be recursive or mutually recursive. Mutually recursive types are declared by a *consecutive* sequence of type declarations, without value or function declarations in the middle. Each recursion cycle must pass through a record or array type.
The following is legal:

```
type intlist = {hd: int, tl: intlist}
type tree = {key: int, children: treelist}
type treelist = {hd: tree, tl: treelist}
```
While the following is not:
```
type b = c
type c = b
```

### 1.2.2 Variable declarations

```
vardec -> 'var' id ':=' exp
       -> 'var' id ':' type-id ':=' exp
```

The first is a short version where the type of the variable is inferred by the type of the expression. In the long form, `type-id` must be the same as the type of the expression to the right of `:=`.
The long form must be used when the initializing expression is `nil`. Each variable declaration creates a new variable which lasts as long as the scope of the declaration.

### 1.2.3 Function declarations

```
fundec -> 'function' id '(' tyfields ')' '=' exp
       -> 'function' id '(' tyfields ')' ':' type-id '=' exp
```

The first is a procedure declaration; the second is a function declaration. Procedures do not return values; functions do, and the type is specified after the colon. The `exp` is the body of the procedure or function, and the `tyfields` specify the names and type of the parameters.
All parameters are passed by value.
Functions may be recursive. Mutually recursive functions and procedures are declared by a sequence of consecutive function declarations (with no type or variable declarations in the middle).

```
function tree_leaves(t: tree): int =
    if t = nil then 1
    else tree_list_leaves(t.children)

function tree_list_leaves(l: treelist): int =
    if l = nil then 0
    else tree_leaves(l.hd) + tree_list_leaves(l.tl)
```
<a id="exp"></a>
## 1.3 Expressions

We use `exp` to denote an expression in the grammar. We also use `exp1`, `exp2` `exp3` do denote `exp` in case we want to reference that particular expression in the discussion.

### 1.3.1 L-values

```
    lvalue -> id
            -> lvalue.id
            -> lvalue[exp]

    exp -> lvalue
```
These are locations that can be read or assigned. Variables, procedure parameters, fields of records, and elements of arrays are all l-values.
The form `id` refers to a variable or parameter accessible by scope rules.
The dot notation allows the selection of the correspondingly named field of a record type.
The bracket notation allows the selection of the correspondingly numbered slot of an array.
Arrays are indexed by consecutive integers starting at zero up to the size of the array minus one.

### 1.3.2 Nil

```
exp -> 'nil'
```

The expression `nil` (reserved word) denotes a value *nil* belonging to every record type. Accessing a field of a *nil* record is undefined. *nil* must be used in a context where its type can be determined. For example:

```
var a : my_record := nil /* OK */
a := nil /* OK */
var a := nil /* Illegal */
if nil = nil then ... /* Illegal */
```

### 1.3.3 Sequencing

```
exp  -> eseq 
eseq -> '(' ε | exp {';' exp} ')'
```
A sequence of two or more expressions, surrounded by parentheses and separated by semicolons evaluates all the expressions in order. The result of the expression is the result (if any) yielded by the last of the expressions. An open parenthesis followed by a close parenthesis is an expression that yields no value.

### 1.3.4 String literals

```
exp -> string
```

A string constant is a sequence, between quotes `"`, of zero or more printable characters, spaces or escape sequences. Each escape sequence is introduced by the escape character `\`, and stands for a character sequence. Here I slightly modify the allowed escape sequences from the Tiger book. They are as follows: `\n`, `\t`, `\ddd` (octal where `d` is a digit),`\"`, `\\`, `\\n`. The last one introduces a multiline string. The string continues with what follows the newline.

### 1.3.5 Integer literals

```
exp -> int
```
A sequence of decimal digits is an integer constant denotes the corresponding integer value.

### 1.3.6 Function calls

```
exp -> id '(' ')'
    -> id '(' exp {',' exp} ')'
```
All the parameters are evaluated left to right. If `id` actually stands for a procedure, then the function body must produce no value, and the function application also produces no value.


### 1.3.7 Negation operator

```
exp -> '-' exp
```

Only an integer-valued expression may be prefixed by a minus sign. 

### 1.3.8 Arithmetic operators

```
exp -> exp op exp
op  -> '+' | '-' | '*' | '/'
```

Such expressions require integer arguments and produce an integer result.

### 1.3.9 Comparison operators

```
exp -> exp op exp
op  -> '=' | '<>' | '>' | '<' | '>=' | '<='
```

Such expressions compare their operands and produce the integer 1 for true and 0 for false. All these operators can be applied to integer operands. The `=` and `<>` operators can also be applied to two record or array operands of the same type, and compare for "pointer" equality (they don't test whether they have the same contents).
All of the above operators can also be applied to strings as well. Two strings are equal if their contents are equal; there is no way to distinguish strings whose component characters are the same. Inequality is according to lexicographic order.

### 1.3.10 Boolean operators

```
exp -> exp op exp
op  -> '&' | '|'
```

These operators apply to integers only.
These are short-circuit boolean conjunctions and disjunctions: they do not evaluate the right-hand operand if the result is determined by the left-hand one. Any nonzero integer value is considered true, and an integer value of zero is false.

### 1.3.11 Assignment operator

```
lvalue ':=' exp
```

The assignment statement evaluates the `lvalue`, then evaluates the `exp`, then sets the contents of the `lvalue` to the result of the expression. Syntactically, `:=` binds weaker than the boolean operators. The assignment operator produces no value.

When an array or record variable `a` is assigned a value `b`, then `a` references the same array or record as `b`. Further updates of elements of `a` will affect `b`, and viceversa, until `a` is reassigned. Parameter passing of arrays and records is by *reference*, not by copying.

### 1.3.12 Precedence of operators

The precedence from highest to lowest is as follows (with operators on the same level having the same precedence):
- unary minus `-`
- `*`, `/`
- `+`, `-`
- `=`, `<>`, `>`, `<`, `>=`, `<=`
- `&`
- `|`
- `:=`

### 1.3.13 Associativity of operators

The operators `*`, `/`, `+`, `-`, `&`, `|` are all left-associative. The comparison operators do not associate, so `a = b = c` is not a legal expression, although `a = (b = c)` is legal.
The assignment operator does not associate.

### 1.3.14 Records

```
exp -> type-id '{' id '=' exp {',' id '=' exp} '}'
exp -> type-id '{' '}'
```

These create a new record instance of type *type-id*. The field names and types of the record expression must match those of the named type, in the same order.

### 1.3.15 Arrays

```
exp  -> type-id '[' exp1 ']' 'of' exp2
```

This expression evaluates `exp1` and `exp2` in that order to find `n`, the number of elements and `v` the initial value. The type `type-id` must be declared as an array type. The result of the expression is a new array of type `type-id`, indexed from `0` to `n-1`, in which each slot is initialized to the value `v`. `n` must be greater than 0.

### 1.3.16 Lifetime of arrays and records

Records and arrays have infinite extent: each record or array value lasts forever, even after control exits from the scope in which it was created.

### 1.3.17 If

```
exp  -> 'if' exp1 'then' exp2
exp  -> 'if' exp1 'then' exp2 'else' exp3
```

Both `if` expressions evaluate the integer `exp1`. If the result is nonzero, in the first case `exp2` (which must produce no value) is evaluated. The entire expression produces no value.
In the second case, it yields the result of evaluating `exp2`, otherwise it yields the result of `exp3`. The expressions `exp2` and `exp3` must have the same type, which is also the type of the entire expression, or both expressions must produce no value.

### 1.3.18 While

```
exp  -> 'while' exp1 'do' exp2
```

This expression evaluates the integer `exp1`. If the result is nonzero, then `exp2` (which must produce no value) is executed, and then the entire while-expression is reevaluated.

### 1.3.19 For

```
exp  -> 'for' id ':=' exp1 'to' exp2 'do' exp3
```

This expression iterates `exp3` over each integer value of `id` between `exp1` and `exp2`. The variable `id` id a new variable implicitely declared by the `for` statement, whose scope covers only `exp3`, and may not be assigned to. The body `exp3` must produce no value. The upper and lower bounds are evaluated only *once*, prior to entering the body of the loop.
If the upper bound is less than the lower, the body is not executed.

### 1.3.20 Break

```
exp  -> 'break'
```

The `break` expression terminates evaluation of the nearest enclosing while-expression or for-expression. A `break` in a procedure `p` cannot terminate a loop in a procedure `q`, even if `p` is nested within `q`. A `break` that is not within a `while` or `for` is illegal.

### 1.3.21 Let

```
expseq ->  ε | exp {';' exp} 
exp    -> 'let' decs 'in' expseq 'end'
```

The `let` expression evaluates the declarations `decs`, binding types, variables, and functions whose scope then extends over the `expseq`. The `expseq` is a sequence of zero or more expressions, separated by semicolons. The result (if any) of the last exp in the sequence is then the result of the entire `let` expression. A `let` expression with nothing between the `in` and `end` yields no value.

<a id="scope"></a>
## 1.4 Scope rules

**Local variables**: in the expression `'let' ... vardec ... 'in' exp 'end'` the scope of the declared variable starts just after its `vardec` and lasts until the `end`.
<br>

**Parameters**: In `'function' id '(' id1 ',' ... ')' '=' exp` the scope of each parameter lasts throughout the function body `exp`.
<br>

**Nested scopes**: The scope of a variable or parameter includes the bodies of any function definitions in that scope.
<br>

**Types**: In the expression `'let' ... tydecs ... 'in' exp 'end'` the scope of a type identifier starts at the beginning of the consecutive sequence of type declarations defining it and lasts until the `end`.
<br>

**Functions**: In the expression `'let' ... funcdecs ... 'in' exp 'end'` the scope of a function identifier starts at the beginning of the consecutive sequence of function declarations defining it and lasts until the `end`.
<br>

**Namespaces**: There are two different name spaces, one for types and one for functions and variables. A type 'a' can be in scope at the same time as a variable 'a' or a function 'a', but functions and variables of the same name cannot both be in scope simultaneously without hiding each other.
<br>

**Local redeclarations**: A variable ort function declaration may be hidden by the redeclaration of the same name in a smaller scope; for example when `f`:

```
function f(v: int) =
    let var v := 6 in
        print(v);
        let var v := 7 in print(v) end;
        print(v);
        let var v := 8 in print(v) end;
        print(v)
    end
```

is applied to `5` it will print `6 7 6 8 6`.
Similarly, type declarations may be hidden by the redeclaration of the same name in a smaller scope. However, no two functions in a sequence of mutually recursive functions may have the same name; and no two types in a sequence of mutually recursive types may have the same name.

<a id="prog"></a>
## 1.5 Programs

Tiger programs do not have arguments: a program is just an expression `exp`.
The following are examples of Tiger programs:

### 1.5.1 Hello World
These are all equivalent:
```
print("Hello World!\n")
```

```
let
    var s := "Hello World!\n"
in
    print(s)
end
```
### 1.5.2 Sieve of Eratosthenes

This will print all prime numbers up to 200.
<br>
It's a bit verbose because there's no standard function that can print integers.
<br>
`int_array` is an array of booleans. `int_array[i]` is true iff `i` is prime.

```
let
  var N := 200
  type int_array = array of int

  function printint(i: int) =
    let function f(i: int) = if i > 0 
        then (f(i/10); print(chr(i - i/10 * 10 + ord("0"))))
    in  if i < 0 then (print("-"); f(-i))
        else if i > 0 then f(i)
        else print("0")
    end

  function get_primes(primes: int_array) =
    (
      primes[0] := 0;
      primes[1] := 0;
      for i := 2 to N do
      (
        if i * i > N then break;
        if primes[i] then
          let var j := i * i in
            while j <= N do
            (
              primes[j] := 0;
              j := j + i
            )
          end
      )
    )
in
  let
    var primes := int_array[N+1] of 1
    function print_array(a: int_array) =
      for j := 0 to N do
          if a[j] then (printint(j); print(" "))
  in
    get_primes(primes);
    print_array(primes)
  end
end
```

### 1.5.3 Book examples

See `tests/book` for more examples from the book.

<a id="sl"></a>

## 1.6 Standard Library

The following functions are predefined:

`function print(s: string)` : Print `s` on standard output.
<br>

`function getchr() : string` : Read a character from standard input; return empty string on end of file.
<br>

`function ord(s: string): int` : Give ASCII value of first character of `s`; yields -1 is `s` is empty string.
<br>

`function chr(i: int): string` : Single-character string from ASCII value `i`; behavior is undefined if `i` is out of range.
<br>

`function concat(s1: string, s2: string): string` : Concatenation of `s1` and `s2`.
<br>

<a id="req"></a>

# 2. Requirements
Currently, the compiler can be compiled for Linux only, with the usual `gcc` supporting c++23.
Additionally, `libgraphviz-dev` is required to compile the compiler with support for graphviz.

<a id="build"></a>

# 3. Build instructions

The supported targets are:
- `x86_64`

To build for a specific target, pass `CONFIG_TARGET_x=1`, where `x` is one of the value listed above to CMake. Eg:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=$PWD/dist -DCONFIG_TARGET_x86_64=1
```

To build with Graphviz support, pass `-DCONFIG_WITH_GRAPHVIZ=1`. Graphviz is used to pretty print graphs while debugging the compiler.


<a id="compile"></a>

# 4. Compilation

Once installed, the compiler directory looks like:
```
.
├── bin
│   └── tigerc
├── lib
│   └── runtime.o
└── tools
    └── driver.sh
```

`tigerc` produces an assembly file as output. `driver.sh` will call the assembler and link the runtime library inside `lib`. For example, to produce executable `example` from `example.tig` use:
```bash
driver.sh example.tig -o example
```
`driver.sh` calls `gcc` assembler and linker.


<a id="todo"></a>

# 5. TODO
- Implement all standard library functions
- Explain how to add support for another target, leveraging `concept`s where it makes sense
- Explain the differences with the standard Tiger language, if any (for example support for some escape sequences, see [The Tiger language (vanilla)](#tiger))