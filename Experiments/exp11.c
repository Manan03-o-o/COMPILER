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