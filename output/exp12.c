#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include<stdlib.h>
#define MAX 100
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
   for (int k = 0; k < C_count; ++k) if (lr1_to_lalr[k] == s) { 
                    ItemSet J; if (!goto_on(&C[k], X, &J)) continue; 
                    // find index of J in C (exact match) 
                    int idx = -1; 
                    for (int m = 0; m < C_count; ++m) { 
                        if (C[m].n == J.n) { 
                            int all=1; 
                            for (int aa=0; aa<J.n; ++aa) { 
                                int f=0; 
                                for (int bb=0; bb<C[m].n; ++bb) 
                                    if (J.items[aa].p==C[m].items[bb].p && 
J.items[aa].dot==C[m].items[bb].dot && J.items[aa].la==C[m].items[bb].la) { f=1; break; } 
                                if (!f) { all=0; break; } 
                            } 
                            if (all) { idx = m; break; } 
                        } 
                    } 
                    if (idx != -1) { 
                        int t = lr1_to_lalr[idx]; 
                        if (is_terminal[X]) { 
                            ACTION[s][X].action = 1; ACTION[s][X].number = t; // shift 
                        } else { 
                            GOTO[s][X] = t; 
                        } 
                    } 
                } 
            } else { 
                // item A->alpha . , for each lookahead la in items -> reduce 
                if (it.p == 0) { 
                    // augmented production S'->S . , la should be $
                         ACTION[s][it.la].action = 3; ACTION[s][it.la].number = 0; // accept 
                } else { 
                    for (int li = 0; li < I->n; ++li) { 
                        // reduce for each lookahead where p and dot match 
                    } 
                    // Actually add reduce for this item's lookahead 
                    ACTION[s][it.la].action = 2; ACTION[s][it.la].number = it.p; // reduce by 
production p 
                } 
            } 
        } 
    } 
} 
 
// Helper to find symbol index by string safely 
int find_sym(const char *s) { 
    for (int i = 0; i < sym_count; ++i) if (strcmp(symbols[i], s) == 0) return i; 
    return -1; 
} 
 
// Parse input tokens (space separated terminals) ending with $ 
void parse_input() { 
    printf("Enter input tokens separated by spaces (terminals). Use 'id' or token names. End 
with $ or include it):\n"); 
    char line[1024]; 
    if (!fgets(line, sizeof(line), stdin)) return; 
    char *tok = strtok(line, " \t\n"); 
    int input[500]; int in_n = 0; 
    while (tok) { 
        int si = find_sym(tok);
           if (si == -1) { 
            printf("Unknown token '%s'\n", tok); return; } 
        input[in_n++] = si; 
        tok = strtok(NULL, " \t\n"); 
    } 
    if (in_n == 0 || symbols[input[in_n-1]][0] != '$') { 
        // add $ 
        int si = find_sym("$"); if (si == -1) si = get_sym_index("$"); input[in_n++] = si; 
    } 
    // stacks 
    int state_stack[1000]; int state_top = 0; state_stack[state_top++] = 0; 
    int sym_stack[1000]; int sym_top = 0; sym_stack[sym_top++] = find_sym("$"); 
    int ip = 0; 
 
    while (1) { 
        int s = state_stack[state_top-1]; 
        int a = input[ip]; 
        Action act = ACTION[s][a]; 
        if (act.action == 1) { 
            // shift 
            state_stack[state_top++] = act.number; 
            sym_stack[sym_top++] = a; 
            ip++; 
            printf("shift to state %d, token %s\n", act.number, symbols[a]); 
        } else if (act.action == 2) { 
            // reduce by prod r 
            int r = act.number; Prod *p = &prods[r]; 
            for (int i = 0; i < p->rhs_len; ++i) { state_top--; sym_top--; } 
            int t = state_stack[state_top-1]; 
            printf("reduce by %s ->", symbols[p->lhs]); 
                 for (int k = 0; k < p->rhs_len; ++k) printf(" %s", symbols[p->rhs[k]]); 
            printf("\n"); 
            int goto_s = GOTO[t][p->lhs]; 
            if (goto_s == -1) { printf("Error: no goto for state %d and symbol %s\n", t, 
symbols[p->lhs]); return; } 
            state_stack[state_top++] = goto_s; 
            sym_stack[sym_top++] = p->lhs; 
        } else if (act.action == 3) { 
            printf("Accept. Input parsed successfully.\n"); 
            return; 
        } else { 
            printf("Error: no action for state %d and symbol %s\n", s, symbols[a]); 
            return; 
        } 
    } 
} 
 
// Read grammar from file "grammar.txt", else use built-in example 
void load_grammar() { 
    FILE *f = fopen("grammar.txt", "r"); 
    if (!f) { 
        // built-in grammar: simple expression grammar 
        // We'll create augmented production 0 later 
        char *lines[] = { 
            "E->E + T", 
            "E->T", 
            "T->T * F", 
            "T->F", 
            "F->( E )", 
            "F->id",
              NULL 
        }; 
        prod_count = 0; 
        for (int i = 0; lines[i]; ++i) { 
            char buf[256]; strcpy(buf, lines[i]); 
            char *arrow = strstr(buf, "->"); 
            if (!arrow) continue; 
            *arrow = '\0'; 
            char *lhs = buf; char *rhs = arrow+2; 
            while (isspace(*lhs)) lhs++; char *end = lhs+strlen(lhs)-1; while (end>lhs && 
isspace(*end)) *end--='\0'; 
            while (isspace(*rhs)) rhs++; end = rhs+strlen(rhs)-1; while (end>rhs && 
isspace(*end)) *end--='\0'; 
            int L = get_sym_index(lhs); 
            // tokenize rhs 
            char *tk = strtok(rhs, " "); 
            prods[prod_count].lhs = L; prods[prod_count].rhs_len = 0; 
            while (tk) { 
                int si = get_sym_index(tk); 
                prods[prod_count].rhs[prods[prod_count].rhs_len++] = si; 
                tk = strtok(NULL, " "); 
            } 
            prod_count++; 
        } 
    } else { 
        char line[256]; 
        prod_count = 0; 
        while (fgets(line, sizeof(line), f)) { 
            char *s = line; while (isspace(*s)) s++; if (*s=='\0' || *s=='#') continue; 
            char *arrow = strstr(s, "->"); if (!arrow) continue; *arrow='\0';
                char *lhs = s; char *rhs = arrow+2; char *end = lhs+strlen(lhs)-1; while (end>lhs && 
isspace(*end)) *end--='\0'; 
            end = rhs+strlen(rhs)-1; while (end>rhs && isspace(*end)) *end--='\0'; 
            int L = get_sym_index(lhs); 
            prods[prod_count].lhs = L; prods[prod_count].rhs_len = 0; 
            char *tk = strtok(rhs, " \t\n"); 
            while (tk) { int si = get_sym_index(tk); 
prods[prod_count].rhs[prods[prod_count].rhs_len++] = si; tk = strtok(NULL, " \t\n"); } 
            prod_count++; 
        } 
        fclose(f); 
    } 
    // add augmented production S' -> S (production 0) 
    // Create new symbol S' 
    char aug[10]; strcpy(aug, "S'"); 
    int sp = get_sym_index(aug); 
    // find original start as first prod's lhs 
    if (prod_count > 0) start_symbol = prods[0].lhs; else start_symbol = get_sym_index("S"); 
    // shift existing productions to make room for augmented production at index 0 
    for (int i = prod_count; i > 0; --i) prods[i] = prods[i-1]; 
    prods[0].lhs = sp; prods[0].rhs_len = 1; prods[0].rhs[0] = start_symbol; 
    prod_count++; 
 
    // mark terminals: symbols that never appear on LHS are terminals (except $) 
    for (int i = 0; i < sym_count; ++i) is_terminal[i] = 1; 
    for (int i = 0; i < prod_count; ++i) is_terminal[prods[i].lhs] = 0; 
    // add $ terminal if not present 
    int dollar = find_sym("$"); if (dollar == -1) { dollar = get_sym_index("$"); 
is_terminal[dollar]=1; } 
}
int main() { 
    load_grammar(); 
    printf("Symbols (%d):\n", sym_count); 
    for (int i = 0; i < sym_count; ++i) printf("  %d: %s (%s)\n", i, symbols[i], 
is_terminal[i]?"T":"N"); 
    printf("Productions (%d):\n", prod_count); 
    for (int i = 0; i < prod_count; ++i) { 
        printf("  %d: %s ->", i, symbols[prods[i].lhs]); 
        for (int j = 0; j < prods[i].rhs_len; ++j) printf(" %s", symbols[prods[i].rhs[j]]); 
        printf("\n"); 
    } 
    compute_first(); 
    printf("Computed FIRST sets.\n"); 
    build_LR1(); 
    printf("Built LR(1) collection with %d states.\n", C_count); 
    build_LALR_from_LR1(); 
    printf("Merged to %d LALR states.\n", lalr_count); 
    build_parsing_table(); 
    printf("Parsing table built.\n"); 
    // optionally print states 
    for (int i = 0; i < lalr_count; ++i) { 
        printf("\nLALR State %d:\n", i); 
        itemset_print(&lalr_states[i]); 
    } 
    // print ACTION table for terminals 
    printf("\nACTION table (terminals and $):\n"); 
    for (int t = 0; t < sym_count; ++t) if (is_terminal[t]) printf("\t%s", symbols[t]); 
    printf("\n"); 
    for (int s = 0; s < lalr_count; ++s) {
          printf("state %d:\t", s); 
        for (int t = 0; t < sym_count; ++t) if (is_terminal[t]) { 
            Action a = ACTION[s][t]; 
            if (a.action == 0) printf("  .  "); 
            else if (a.action == 1) printf(" s%d ", a.number); 
            else if (a.action == 2) printf(" r%d ", a.number); 
            else if (a.action == 3) printf(" acc "); 
            printf("\t"); 
        } 
        printf("\n"); 
    } 
 
    parse_input(); 
    return 0; 
}