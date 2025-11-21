 
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
typedef struct { int lhs; int rhs[MAX_RHS]; int rhs_len; } Prod; 
Prod prods[MAX_PROD]; 
int prod_count = 0; 
 
// Augmented grammar: S'->S 
int start_symbol = -1; 
 
// LR(1) item: production index, dot position, lookahead symbol 
typedef struct { int p; int dot; int la; } Item; 
 
// Item set 
typedef struct { 
    Item items[MAX_ITEMS]; 
    int n; 
} ItemSet; 
 
ItemSet C[MAX_STATES]; 
int C_count = 0; 
 
// Helpers 
int contains_item(const ItemSet *s, Item it) { 
    for (int i = 0; i < s->n; ++i) 
        if (s->items[i].p == it.p && s->items[i].dot == it.dot && s->items[i].la == it.la) return 1; 
    return 0; 
} 
 
int items_equal_core(const ItemSet *a, const ItemSet *b) { 
    // cores equal if p and dot equal for every item disregarding lookahead, and sizes match 
    // We'll check that every core item in a exists in b and vice versa 
    for (int i = 0; i < a->n; ++i) {
             int found = 0; 
        for (int j = 0; j < b->n; ++j) if (a->items[i].p == b->items[j].p && a->items[i].dot == b
>items[j].dot) { found = 1; break; } 
        if (!found) return 0; 
    } 
    for (int i = 0; i < b->n; ++i) { 
        int found = 0; 
        for (int j = 0; j < a->n; ++j) if (b->items[i].p == a->items[j].p && b->items[i].dot == a
>items[j].dot) { found = 1; break; } 
        if (!found) return 0; 
    } 
    return 1; 
} 
 
// FIRST sets 
int FIRST[MAX_SYMBOLS][MAX_FIRST]; // store symbol indices 
int FIRST_count[MAX_SYMBOLS]; 
int FIRST_epsilon[MAX_SYMBOLS]; 
 
void add_first(int X, int t) { 
    for (int i = 0; i < FIRST_count[X]; ++i) if (FIRST[X][i] == t) return; 
    FIRST[X][FIRST_count[X]++] = t; 
} 
 
void compute_first() { 
    // Initialize 
    for (int i = 0; i < sym_count; ++i) { FIRST_count[i]=0; FIRST_epsilon[i]=0; } 
    // terminals: FIRST(a) = {a} 
    for (int i = 0; i < sym_count; ++i) if (is_terminal[i]) add_first(i, i);
       int changed = 1; 
    while (changed) { 
        changed = 0; 
        for (int pi = 0; pi < prod_count; ++pi) { 
            Prod *p = &prods[pi]; 
            int A = p->lhs; 
            int can_eps = 1; 
            for (int k = 0; k < p->rhs_len; ++k) { 
                int B = p->rhs[k]; 
                // add FIRST(B) \ {epsilon} to FIRST(A) 
                for (int j = 0; j < FIRST_count[B]; ++j) { 
                    int t = FIRST[B][j]; 
                    if (t == -1) continue; 
                    int before = FIRST_count[A]; 
                    add_first(A, t); 
                    if (FIRST_count[A] != before) changed = 1; 
                } 
                if (!FIRST_epsilon[B]) { can_eps = 0; break; } 
            } 
            if (can_eps && !FIRST_epsilon[A]) { FIRST_epsilon[A]=1; changed=1; } 
        } 
    } 
} 
 
// FIRST sequence: for sequence X1 X2 ... compute FIRST 
void first_of_sequence(int seq[], int len, int out[], int *outn) { 
    *outn = 0; 
    int can_eps = 1; 
    for (int i = 0; i < len; ++i) { 
        int X = seq[i];