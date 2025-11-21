 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <ctype.h> 
 
#define MAX_PROD 100 
#define MAX_SYMBOLS 100 
#define MAX_LEN 256 
#define MAX_ITEMS 2000 
#define MAX_STATES 500 
#define MAX_RHS 20 
#define MAX_ACTIONS 5000 
#define MAX_FIRST 200 
 
// Symbol table 
char symbols[MAX_SYMBOLS][MAX_LEN]; 
int sym_count = 0; 
 
int is_terminal[MAX_SYMBOLS]; // 1 if terminal, 0 if nonterminal 
 
int get_sym_index(const char *s) { 
    for (int i = 0; i < sym_count; ++i) if (strcmp(symbols[i], s) == 0) return i; 
    strcpy(symbols[sym_count], s); 
    is_terminal[sym_count] = -1; // unknown yet 
    return sym_count++; 
} 
 
// Production structure