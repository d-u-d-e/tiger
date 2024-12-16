#include <stdio.h>
extern "C"
{
    extern int yylex();
    extern FILE *yyin;
}

int main(int argc, char **argv)
{
    ++argc;
    --argv;
    if (argc > 0)
        yyin = fopen(argv[0], "r");
    else
        yyin = stdin;
    yylex();
    return 0;
}
