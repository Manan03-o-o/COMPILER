 
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
        for (int j = 0; j < FIRST_count[X]; ++j) { 
            int t = FIRST[X][j]; 
            if (t == -1) continue; 
            int exists = 0; 
            for (int k = 0; k < *outn; ++k) if (out[k] == t) { exists = 1; break; } 
            if (!exists) out[(*outn)++] = t; 
        } 
        if (!FIRST_epsilon[X]) { can_eps = 0; break; } 
    } 
    if (can_eps) { /* add epsilon if needed: represented by -1 */ 
        int exists = 0; for (int k=0;k<*outn;++k) if (out[k]==-1) exists=1; 
        if (!exists) out[(*outn)++] = -1; 
    } 
} 
 
// Closure and goto 
 
void closure(ItemSet *I) { 
    int changed = 1; 
    while (changed) { 
        changed = 0; 
        for (int i = 0; i < I->n; ++i) { 
            Item it = I->items[i]; 
            Prod *p = &prods[it.p]; 
            if (it.dot < p->rhs_len) { 
                int B = p->rhs[it.dot]; 
                if (!is_terminal[B]) { 
                    // compute lookahead set: beta a 
                    int beta[MAX_RHS]; int bl = 0; 
                    for (int k = it.dot+1; k < p->rhs_len; ++k) beta[bl++]=p->rhs[k]; 
   beta[bl++] = it.la; 
                    int la_set[MAX_FIRST]; int lan=0; 
                    first_of_sequence(beta, bl, la_set, &lan); 
                    for (int prod_j = 0; prod_j < prod_count; ++prod_j) { 
                        if (prods[prod_j].lhs == B) { 
                            for (int li = 0; li < lan; ++li) { 
                                int la = la_set[li]; if (la == -1) continue; // epsilon as lookahead not useful 
                                Item newit; newit.p = prod_j; newit.dot = 0; newit.la = la; 
                                if (!contains_item(I, newit)) { I->items[I->n++] = newit; changed = 1; } 
                            } 
                        } 
                    } 
                } 
            } 
        } 
    } 
} 
 
void itemset_print(const ItemSet *I) { 
    printf("Items (n=%d):\n", I->n); 
    for (int i = 0; i < I->n; ++i) { 
        Item it = I->items[i]; 
        Prod *p = &prods[it.p]; 
        printf("  (%d) %s -> ", it.p, symbols[p->lhs]); 
        for (int k = 0; k < p->rhs_len; ++k) { 
            if (k == it.dot) printf(" . "); 
            printf("%s ", symbols[p->rhs[k]]); 
        } 
        if (it.dot == p->rhs_len) printf(" . "); 
        printf(", la=%s\n", symbols[it.la]);
            } 
} 
 
int goto_on(const ItemSet *I, int X, ItemSet *J) { 
    J->n = 0; 
    for (int i = 0; i < I->n; ++i) { 
        Item it = I->items[i]; 
        Prod *p = &prods[it.p]; 
        if (it.dot < p->rhs_len && p->rhs[it.dot] == X) { 
            Item nit = it; nit.dot++; 
            // add nit 
            if (!contains_item(J, nit)) J->items[J->n++] = nit; 
        } 
    } 
    if (J->n == 0) return 0; 
    closure(J); 
    return 1; 
} 
 
// State merging: find state with same core 
int find_core_equal_state(const ItemSet *s) { 
    for (int i = 0; i < C_count; ++i) if (items_equal_core(&C[i], s)) return i; 
    return -1; 
} 
 
// Build canonical LR(1) collection and then merge cores to LALR 
 
void build_LR1() { 
    // initial item: augmented production is prods[0] 
    C_count = 0;
     ItemSet I0; I0.n = 0; 
    // production 0 should be S'->S 
    Item it0 = {0, 0, get_sym_index("$")}; // $ as lookahead 
    I0.items[I0.n++] = it0; 
    closure(&I0); 
    C[C_count++] = I0; 
    int changed = 1; 
    while (changed) { 
        changed = 0; 
        for (int i = 0; i < C_count; ++i) { 
            // for all grammar symbols 
            for (int X = 0; X < sym_count; ++X) { 
                ItemSet J; if (!goto_on(&C[i], X, &J)) continue; 
                // check if J exists 
                int exists = -1; 
                for (int k = 0; k < C_count; ++k) { 
                    if (C[k].n == J.n) { 
                        int all = 1; 
                        for (int a = 0; a < J.n; ++a) { 
                            int found = 0; 
                            for (int b = 0; b < C[k].n; ++b) 
                                if (J.items[a].p == C[k].items[b].p && J.items[a].dot == 
C[k].items[b].dot && J.items[a].la == C[k].items[b].la) { found=1; break; } 
                            if (!found) { all=0; break; } 
                        } 
                        if (all) { exists = k; break; } 
                    } 
                } 
                if (exists == -1) { C[C_count++] = J; changed = 1; } 
            }
                 } 
    } 
} 
 
// We'll merge states with same core to form LALR: map LR1 states -> LALR state index 
int lr1_to_lalr[MAX_STATES]; 
ItemSet lalr_states[MAX_STATES]; 
int lalr_count = 0; 
 
void build_LALR_from_LR1() { 
    memset(lr1_to_lalr, -1, sizeof(lr1_to_lalr)); 
    lalr_count = 0; 
    for (int i = 0; i < C_count; ++i) { 
        if (lr1_to_lalr[i] != -1) continue; 
        // create new LALR state with core = C[i] core 
        lalr_states[lalr_count] = C[i]; 
        lr1_to_lalr[i] = lalr_count; 
        // merge lookaheads from other LR(1) states with equal core 
        for (int j = i+1; j < C_count; ++j) { 
            if (items_equal_core(&C[i], &C[j])) { 
                // add items from C[j] whose (p,dot) not already present 
                for (int a = 0; a < C[j].n; ++a) { 
                    Item it = C[j].items[a]; 
                    int found = 0; 
                    for (int b = 0; b < lalr_states[lalr_count].n; ++b) 
                        if (lalr_states[lalr_count].items[b].p == it.p && 
lalr_states[lalr_count].items[b].dot == it.dot && lalr_states[lalr_count].items[b].la == it.la) { 
found=1; break; } 
                    if (!found) lalr_states[lalr_count].items[lalr_states[lalr_count].n++] = it; 
                }
                  lr1_to_lalr[j] = lalr_count; 
            } 
        } 
        lalr_count++; 
    } 
} 
 
// Parsing table: ACTION and GOTO 
// ACTION: for terminals and $, use encoding: shift s to state, reduce by prod r, accept, or 
error 
typedef struct { int action; int number; } Action; // action: 0=error,1=shift,2=reduce,3=accept 
 
Action ACTION[MAX_STATES][MAX_SYMBOLS]; 
int GOTO[MAX_STATES][MAX_SYMBOLS]; 
 
void build_parsing_table() { 
    // initialize 
    for (int i = 0; i < lalr_count; ++i) for (int j = 0; j < sym_count; ++j) { ACTION[i][j].action 
= 0; ACTION[i][j].number = -1; GOTO[i][j] = -1; } 
    // for each LALR state, for each item 
    for (int s = 0; s < lalr_count; ++s) { 
        ItemSet *I = &lalr_states[s]; 
        // shifts and gotos: for items A->alpha . X beta, goto on X to t 
        for (int a = 0; a < I->n; ++a) { 
            Item it = I->items[a]; Prod *p = &prods[it.p]; 
            if (it.dot < p->rhs_len) { 
                int X = p->rhs[it.dot]; 
                // find LR1 goto from any LR1 state mapped to s 
                // We'll search in original LR(1) collection for state k mapped to s, compute 
goto_on(C[k], X) to J, find which LR1 state index matches J 
