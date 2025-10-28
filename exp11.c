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
 } else { 
        printf("Applying Rule: T' -> e\n"); 
    } 
} 
 
// F -> ( E ) | id 
void F() { 
    if (input[i] == '(') { 
        printf("Applying Rule: F -> ( E )\n"); 
        match('('); 
        E(); 
        match(')'); 
    } else if (isalpha(input[i])) { 
        printf("Applying Rule: F -> id\n"); 
        printf("Matched identifier '%c'\n", input[i]); 
        match(input[i]);  // Match the single-letter id 
    } else { 
        error(); 
    } 
} 
 
int main() { 
    printf("Enter an arithmetic expression (e.g., a+b*c): "); 
    scanf("%s", input); 
 
    printf("\n--- Parsing Started ---\n"); 
    E(); 
 
    if (input[i] == '\0') { 
        printf("--- Parsing Completed Successfully ---\n");
          printf("Input accepted\n"); 
    } else { 
        printf("Unexpected character '%c' at end\n", input[i]); 
        printf("Input rejected\n"); 
    } 
 
    return 0; 
}