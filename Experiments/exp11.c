#include <stdio.h> 
#include <ctype.h>
#include <string.h> 
#include <stdlib.h> 
 
char input[100]; 
int i = 0; 
 
void E(); 
void Eprime(); 
void T(); 
void Tprime(); 
void F(); 
 
void error() { 
    printf("Syntax Error at position %d (near '%c')\n", i, input[i]); 
    exit(1); 
} 
 
void match(char expected) { 
    if (input[i] == expected) { 
        printf("Matched '%c'\n", expected); 
        i++; 
    } else { 
        printf("Expected '%c' but found '%c'\n", expected, input[i]); 
        error(); 
    } 
} 
 
// E -> T E' 
void E() { 
    printf("Applying Rule: E -> T E'\n");
       T(); 
    Eprime(); 
} 
 
// E' -> + T E' | e 
void Eprime() { 
    if (input[i] == '+') { 
        printf("Applying Rule: E' -> + T E'\n"); 
        match('+'); 
        T(); 
        Eprime(); 
    } else { 
        printf("Applying Rule: E' -> e\n"); 
    } 
} 
 
// T -> F T' 
void T() { 
    printf("Applying Rule: T -> F T'\n"); 
    F(); 
    Tprime(); 
} 
 
// T' -> * F T' | e 
void Tprime() { 
    if (input[i] == '*') { 
        printf("Applying Rule: T' -> * F T'\n"); 
        match('*'); 
        F(); 
        Tprime();