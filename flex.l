
%{
#include <stdio.h>
%}

ID [a-zA-Z]+
SIGN [-|+]
DIGIT [0-9]
EXP [e|E][0-9]+

%option noyywrap

%%

{ID} { printf("IDENTIFICATORE\n"); }

{SIGN}?{DIGIT}+ { printf("INTERO\n"); }

{SIGN}?{DIGIT}*"."{DIGIT}+{EXP}? { printf("REALE\n"); }

[ \t\r\n]+

%%