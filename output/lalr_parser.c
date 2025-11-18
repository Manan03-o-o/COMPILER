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
#define MAX_PROD 500 // FIX 1: Added missing definition

// Symbol table 
char symbols[MAX_SYMBOLS][MAX_LEN]; 
int sym_count = 0; 
 
int is_terminal[MAX_SYMBOLS]; // 1 if terminal, 0 if nonterminal 
 
int get_sym_index(const char *s) { 
    for (int i = 0; i < sym_count; ++i) if (strcmp(symbols[i], s) == 0) return i; 
    if (sym_count >= MAX_SYMBOLS) { printf("Error: Symbol table full.\n"); exit(1); }
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
    // cores equal if p and dot equal for every item disregarding lookahead
    // Check that every core item in a exists in b
    for (int i = 0; i < a->n; ++i) { 
        int found = 0; 
        for (int j = 0; j < b->n; ++j) {
            if (a->items[i].p == b->items[j].p && a->items[i].dot == b->items[j].dot) { 
                found = 1; 
                break; 
            }
        }
        if (!found) return 0; 
    } 
    // Check that every core item in b exists in a
    for (int i = 0; i < b->n; ++i) { 
        int found = 0; 
        for (int j = 0; j < a->n; ++j) {
            if (b->items[i].p == a->items[j].p && b->items[i].dot == a->items[j].dot) { 
                found = 1; 
                break; 
            }
        }
        if (!found) return 0; 
    } 
    return 1; 
} 
 
// FIRST sets 
int FIRST[MAX_SYMBOLS][MAX_FIRST]; // store symbol indices 
int FIRST_count[MAX_SYMBOLS]; 
int FIRST_epsilon[MAX_SYMBOLS]; 
 
void add_first(int X, int t) { 
    if (FIRST_count[X] >= MAX_FIRST) { printf("Error: FIRST set full.\n"); return; }
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
                    if (t == -1) continue; // Should not happen with symbol indices
                    int before = FIRST_count[A]; 
                    add_first(A, t); 
                    if (FIRST_count[A] != before) changed = 1; 
                } 
                if (!FIRST_epsilon[B]) { can_eps = 0; break; } 
            } 
            // Handle A -> epsilon (empty RHS)
            if (p->rhs_len == 0 && !FIRST_epsilon[A]) {
                FIRST_epsilon[A] = 1; changed = 1;
            }
            if (can_eps && p->rhs_len > 0 && !FIRST_epsilon[A]) { FIRST_epsilon[A]=1; changed=1; } 
        } 
    } 
} 
 
// FIRST sequence: for sequence X1 X2 ... compute FIRST 
void first_of_sequence(int seq[], int len, int out[], int *outn, int *has_eps) { 
    *outn = 0; 
    *has_eps = 1; // Assume epsilon until proven otherwise
    for (int i = 0; i < len; ++i) { 
        int X = seq[i]; 
        if (X == -1) continue; // Skip placeholder
        for (int j = 0; j < FIRST_count[X]; ++j) { 
            int t = FIRST[X][j]; 
            int exists = 0; 
            for (int k = 0; k < *outn; ++k) if (out[k] == t) { exists = 1; break; } 
            if (!exists) out[(*outn)++] = t; 
        } 
        if (!FIRST_epsilon[X]) { *has_eps = 0; break; } 
    } 
} 
 
// Closure and goto 
 
void closure(ItemSet *I) { 
    int changed = 1; 
    while (changed) { 
        changed = 0; 
        for (int i = 0; i < I->n; ++i) { 
            if (I->n >= MAX_ITEMS) { printf("Error: Item set full.\n"); return; }
            Item it = I->items[i]; 
            Prod *p = &prods[it.p]; 
            if (it.dot < p->rhs_len) { 
                int B = p->rhs[it.dot]; 
                if (!is_terminal[B]) { 
                    // compute lookahead set: FIRST(beta a) 
                    int beta[MAX_RHS]; int bl = 0; 
                    for (int k = it.dot+1; k < p->rhs_len; ++k) beta[bl++]=p->rhs[k];
                    beta[bl++] = it.la; // Add 'a' to the end of beta
                    
                    int la_set[MAX_FIRST]; int lan=0; int has_eps = 0;
                    first_of_sequence(beta, bl, la_set, &lan, &has_eps); 
                    
                    for (int prod_j = 0; prod_j < prod_count; ++prod_j) { 
                        if (prods[prod_j].lhs == B) { 
                            for (int li = 0; li < lan; ++li) { 
                                int la = la_set[li]; 
                                Item newit; newit.p = prod_j; newit.dot = 0; newit.la = la; 
                                if (!contains_item(I, newit)) { 
                                    if (I->n >= MAX_ITEMS) { printf("Error: Item set full (inner).\n"); exit(1); }
                                    I->items[I->n++] = newit; 
                                    changed = 1; 
                                } 
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
        printf("  (%d) %s -> ", it.p, symbols[p->lhs]); 
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
            if (!contains_item(J, nit)) {
                if (J->n >= MAX_ITEMS) { printf("Error: GOTO Item set full.\n"); exit(1); }
                J->items[J->n++] = nit;
            } 
        } 
    } 
    if (J->n == 0) return 0; 
    closure(J); 
    return 1; 
} 
 
// State merging: find state with same core 
// (This function is unused in the current logic)
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
    if (I0.n >= MAX_ITEMS) { printf("Error: Item set full.\n"); exit(1); }
    I0.items[I0.n++] = it0; 
    closure(&I0); 
    if (C_count >= MAX_STATES) { printf("Error: State table full.\n"); exit(1); }
    C[C_count++] = I0; 
    
    int processed = 0; // Use a processed index instead of 'changed'
    while (processed < C_count) {
        int i = processed++; // Process state i
        if (i >= MAX_STATES) { printf("Error: State limit reached.\n"); break; }
        
        // for all grammar symbols 
        for (int X = 0; X < sym_count; ++X) { 
            if (strcmp(symbols[X], "S'") == 0) continue; // Don't goto on S'

            ItemSet J; 
            if (!goto_on(&C[i], X, &J)) continue; 
            
            // check if J exists 
            int exists = -1; 
            for (int k = 0; k < C_count; ++k) { 
                if (C[k].n == J.n) { 
                    int all = 1; 
                    for (int a = 0; a < J.n; ++a) { 
                        if (!contains_item(&C[k], J.items[a])) {
                            all = 0;
                            break;
                        }
                    } 
                    if (all) { exists = k; break; } 
                } 
            } 
            
            if (exists == -1) { 
                if (C_count >= MAX_STATES) { printf("Error: State table full (inner).\n"); exit(1); }
                C[C_count++] = J; 
            } 
        }
        // FIX 3: Removed extra closing brace that was here
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
        
        // create new LALR state
        if (lalr_count >= MAX_STATES) { printf("Error: LALR state table full.\n"); exit(1); }
        lalr_states[lalr_count] = C[i]; 
        lr1_to_lalr[i] = lalr_count; 
        
        // merge lookaheads from other LR(1) states with equal core 
        for (int j = i+1; j < C_count; ++j) { 
            if (lr1_to_lalr[j] != -1) continue; // Already merged
            if (items_equal_core(&C[i], &C[j])) { 
                // add items from C[j] that are not already present
                for (int a = 0; a < C[j].n; ++a) { 
                    Item it = C[j].items[a]; 
                    if (!contains_item(&lalr_states[lalr_count], it)) {
                        if (lalr_states[lalr_count].n >= MAX_ITEMS) { printf("Error: LALR item set full.\n"); exit(1); }
                        lalr_states[lalr_count].items[lalr_states[lalr_count].n++] = it;
                    }
                }
                lr1_to_lalr[j] = lalr_count; 
            } 
        } 
        lalr_count++; 
    } 
} 
 
// Parsing table: ACTION and GOTO 
typedef struct { int action; int number; } Action; // action: 0=error,1=shift,2=reduce,3=accept 
 
Action ACTION[MAX_STATES][MAX_SYMBOLS]; 
int GOTO[MAX_STATES][MAX_SYMBOLS]; 
 
// FIX 4: Re-wrote build_parsing_table to handle conflicts
void build_parsing_table() { 
    // initialize 
    for (int i = 0; i < lalr_count; ++i) {
        for (int j = 0; j < sym_count; ++j) { 
            ACTION[i][j].action = 0; 
            ACTION[i][j].number = -1; 
            GOTO[i][j] = -1; 
        }
    }
    
    // for each LALR state
    for (int s = 0; s < lalr_count; ++s) { 
        ItemSet *I = &lalr_states[s]; 
        
        // Handle shifts and gotos
        for (int a = 0; a < I->n; ++a) { 
            Item it = I->items[a]; 
            Prod *p = &prods[it.p]; 
            
            if (it.dot < p->rhs_len) { 
                int X = p->rhs[it.dot]; 
                
                // Find the GOTO state. We must re-calculate the LR(1) goto
                // from an original LR(1) state 'k' that maps to LALR state 's'.
                int k = -1;
                for(int i=0; i<C_count; ++i) if(lr1_to_lalr[i] == s) { k = i; break; }
                if (k == -1) continue; // Should not happen

                ItemSet J; 
                // We only need the goto from *one* of the original cores
                // But we must use the *original* item set C[k] to compute goto
                ItemSet K_goto; K_goto.n = 0;
                for (int i = 0; i < C[k].n; ++i) { 
                    Item it_k = C[k].items[i]; 
                    Prod *p_k = &prods[it_k.p]; 
                    if (it_k.dot < p_k->rhs_len && p_k->rhs[it_k.dot] == X) { 
                        Item nit = it_k; nit.dot++; 
                        if (!contains_item(&K_goto, nit)) K_goto.items[K_goto.n++] = nit; 
                    } 
                } 
                if (K_goto.n == 0) continue; 
                closure(&K_goto);
                
                // Find which LR(1) state K_goto matches
                int idx = -1; 
                for (int m = 0; m < C_count; ++m) { 
                    if (C[m].n == K_goto.n && contains_item(&C[m], K_goto.items[0])) {
                        int all=1;
                        for(int aa=0; aa<K_goto.n; ++aa) if(!contains_item(&C[m], K_goto.items[aa])) all=0;
                        if(all) { idx = m; break; }
                    }
      S         } 

                if (idx != -1) { 
                    int t = lr1_to_lalr[idx]; // The target LALR state
                    if (is_terminal[X]) { 
                        // Check for conflict
                        if (ACTION[s][X].action == 2) { // Shift/Reduce conflict
                            printf("Conflict (S/R): State %d, %s: Reduce r%d vs Shift s%d. Resolving in favor of SHIFT.\n", s, symbols[X], ACTION[s][X].number, t);
                        } else if (ACTION[s][X].action == 1 && ACTION[s][X].number != t) {
                            printf("Warning (S/S): State %d, %s: s%d vs s%d. (Using first)\n", s, symbols[X], ACTION[s][X].number, t);
                        }
                        // Set shift action (overwriting reduce if conflict)
                        ACTION[s][X].action = 1; 
                        ACTION[s][X].number = t; 
                    } else { 
                        GOTO[s][X] = t; 
                    } 
                } 
            } 
        }
        
        // Handle reductions
        for (int a = 0; a < I->n; ++a) {
            Item it = I->items[a]; 
            Prod *p = &prods[it.p]; 
            
            if (it.dot == p->rhs_len) { // Reduce item A -> alpha .
                int la = it.la;
                if (it.p == 0) { 
                    // augmented production S'->S . , la should be $
                    ACTION[s][la].action = 3; 
                    ACTION[s][la].number = 0; // accept 
                } else { 
                    // FIX 5: Removed empty 'for' loop that was here
                    // Check for conflicts
                    if (ACTION[s][la].action == 1) { // Shift/Reduce conflict
                        // S/R Conflict. We already printed and resolved (in favor of shift)
                        // so we do nothing here.
                    } else if (ACTION[s][la].action == 2) { // Reduce/Reduce conflict
                        if (ACTION[s][la].number != it.p) {
                            printf("Conflict (R/R): State %d, %s: Reduce r%d vs Reduce r%d. Using r%d.\n", s, symbols[la], ACTION[s][la].number, it.p, ACTION[s][la].number);
                        }
                    } else if (ACTION[s][la].action == 0) {
                        // No conflict, add reduce action
                        ACTION[s][la].action = 2; 
                        ACTION[s][la].number = it.p; // reduce by production p
                    }
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
    printf("Enter input tokens separated by spaces (terminals). Use 'id' or token names. End with $ or include it automatically:\n"); 
    char line[1024]; 
    if (!fgets(line, sizeof(line), stdin)) return; 
    char *tok = strtok(line, " \t\n"); 
    int input[500]; int in_n = 0; 
    int dollar_idx = find_sym("$");
    if (dollar_idx == -1) { // Should be added by load_grammar
        dollar_idx = get_sym_index("$");
        is_terminal[dollar_idx] = 1;
    }

    while (tok) { 
        int si = find_sym(tok);
        if (si == -1) { 
            printf("Unknown token '%s'\n", tok); return; 
        }
        if (si == dollar_idx) break; // Stop if $ is found
        if (in_n >= 499) { printf("Input too long.\n"); return; }
        input[in_n++] = si; 
        tok = strtok(NULL, " \t\n"); 
    } 
    input[in_n++] = dollar_idx; // Add $
    
    // stacks 
    int state_stack[1000]; int state_top = 0; state_stack[state_top++] = 0; 
    int sym_stack[1000]; int sym_top = 0; // sym_stack is for debugging, not strictly needed
    
    int ip = 0; 
 
    printf("\nParsing steps:\n");
    while (1) { 
        int s = state_stack[state_top-1]; 
        int a = input[ip]; 
        Action act = ACTION[s][a]; 
        
        if (act.action == 1) { 
            // shift 
            printf("State %d, Token '%s': Shift to state %d\n", s, symbols[a], act.number); 
            if (state_top >= 1000) { printf("Stack overflow.\n"); return; }
            state_stack[state_top++] = act.number; 
            sym_stack[sym_top++] = a; // for debug
            ip++; 
        } else if (act.action == 2) { 
            // reduce by prod r 
            int r = act.number; 
            Prod *p = &prods[r]; 
            printf("State %d, Token '%s': Reduce by %s ->", s, symbols[a], symbols[p->lhs]);
            for (int k = 0; k < p->rhs_len; ++k) printf(" %s", symbols[p->rhs[k]]); 
            printf(" (prod %d)\n", r);
            
            if (state_top < p->rhs_len) { printf("Stack underflow on reduce.\n"); return; }
            state_top -= p->rhs_len;
            sym_top -= p->rhs_len; // for debug
            
            int t = state_stack[state_top-1]; 
            int goto_s = GOTO[t][p->lhs]; 
            if (goto_s == -1) { 
                printf("Error: no goto for state %d and symbol %s\n", t, symbols[p->lhs]); 
                return; 
            } 
            printf("   GOTO(state %d, %s) = state %d\n", t, symbols[p->lhs], goto_s);
            if (state_top >= 1000) { printf("Stack overflow.\n"); return; }
            state_stack[state_top++] = goto_s; 
            sym_stack[sym_top++] = p->lhs; // for debug
        } else if (act.action == 3) { 
            printf("State %d, Token '%s': Accept.\n", s, symbols[a]);
            printf("\nInput parsed successfully.\n"); 
            return; 
        } else { 
            printf("Error: No action for state %d and symbol %s\n", s, symbols[a]); 
            // Print expected tokens
            printf("   Expected: ");
            for(int t=0; t<sym_count; ++t) {
                if(is_terminal[t] && ACTION[s][t].action != 0) printf("'%s' ", symbols[t]);
            }
            printf("\n");
            return; 
        } 
    } 
} 
 
// Read grammar from file "grammar.txt", else use built-in example 
void load_grammar() { 
    FILE *f = fopen("grammar.txt", "r"); 
    if (!f) { 
        printf("grammar.txt not found, using built-in example grammar.\n");
        // built-in grammar: simple expression grammar 
        char *lines[] = { 
            "E -> E + T", 
            "E -> T", 
            "T -> T * F", 
            "T -> F", 
            "F -> ( E )", 
            "F -> id",
            NULL 
        }; 
        prod_count = 0; 
        for (int i = 0; lines[i]; ++i) { 
            if (prod_count >= MAX_PROD-1) { printf("Grammar too large.\n"); break; }
            char buf[256]; strcpy(buf, lines[i]); 
            char *arrow = strstr(buf, "->"); 
            if (!arrow) continue; 
            *arrow = '\0'; 
            char *lhs = buf; char *rhs = arrow+2; 
            while (isspace(*lhs)) lhs++; char *end = lhs+strlen(lhs)-1; while (end>lhs && isspace(*end)) *end--='\0'; 
            while (isspace(*rhs)) rhs++; end = rhs+strlen(rhs)-1; while (end>rhs && isspace(*end)) *end--='\0'; 
            
            int L = get_sym_index(lhs); 
            // tokenize rhs 
            char *tk = strtok(rhs, " "); 
            prods[prod_count].lhs = L; prods[prod_count].rhs_len = 0; 
            while (tk) { 
                int si = get_sym_index(tk); 
                if (prods[prod_count].rhs_len >= MAX_RHS) { printf("RHS too long.\n"); break; }
                prods[prod_count].rhs[prods[prod_count].rhs_len++] = si; 
                tk = strtok(NULL, " "); 
            } 
            prod_count++; 
        } 
    } else { 
        printf("Loading grammar from grammar.txt...\n");
        char line[256]; 
        prod_count = 0; 
        while (fgets(line, sizeof(line), f)) { 
            if (prod_count >= MAX_PROD-1) { printf("Grammar too large.\n"); break; }
            char *s = line; while (isspace(*s)) s++; if (*s=='\0' || *s=='#') continue; 
            char *arrow = strstr(s, "->"); if (!arrow) continue; *arrow='\0';
            char *lhs = s; char *rhs = arrow+2; char *end = lhs+strlen(lhs)-1; while (end>lhs && isspace(*end)) *end--='\0'; 
            end = rhs+strlen(rhs)-1; while (end>rhs && isspace(*end)) *end--='\0'; 
            
            int L = get_sym_index(lhs); 
            prods[prod_count].lhs = L; prods[prod_count].rhs_len = 0; 
            char *tk = strtok(rhs, " \t\n"); 
            while (tk) { 
                int si = get_sym_index(tk); 
                if (prods[prod_count].rhs_len >= MAX_RHS) { printf("RHS too long.\n"); break; }
                prods[prod_count].rhs[prods[prod_count].rhs_len++] = si; 
                tk = strtok(NULL, " \t\n"); 
            } 
            prod_count++; 
        } 
        fclose(f); 
    } 
    
    // add augmented production S' -> S (production 0) 
    // Create new symbol S' 
    char aug[MAX_LEN]; // FIX 2: Declared 'aug' array
    strcpy(aug, "S_prime"); // Use a name that won't conflict if 'S'' is used
    int sp = get_sym_index(aug); 
    
    // find original start as first prod's lhs 
    if (prod_count > 0) start_symbol = prods[0].lhs; 
    else {
        printf("No productions loaded. Exiting.\n");
        exit(1);
    }
    
    // shift existing productions to make room for augmented production at index 0 
    for (int i = prod_count; i > 0; --i) prods[i] = prods[i-1]; 
    prods[0].lhs = sp; 
    prods[0].rhs_len = 1; 
    prods[0].rhs[0] = start_symbol; 
    prod_count++; 
 
    // mark terminals: symbols that never appear on LHS are terminals 
    for (int i = 0; i < sym_count; ++i) {
        if (is_terminal[i] == -1) is_terminal[i] = 1; // Default to terminal
    }
    for (int i = 0; i < prod_count; ++i) {
        is_terminal[prods[i].lhs] = 0; // It's a non-terminal
    }
    
    // add $ terminal if not present 
    int dollar = find_sym("$"); 
    if (dollar == -1) dollar = get_sym_index("$"); 
    is_terminal[dollar] = 1;
    is_terminal[sp] = 0; // S' is non-terminal
}
int main() { 
    load_grammar(); 
    
    printf("Symbols (%d):\n", sym_count); 
    for (int i = 0; i < sym_count; ++i) printf("  %d: %s (%s)\n", i, symbols[i], is_terminal[i]?"T":"N"); 
    
    printf("\nProductions (%d):\n", prod_count); 
    for (int i = 0; i < prod_count; ++i) { 
        printf("  %d: %s ->", i, symbols[prods[i].lhs]); 
        for (int j = 0; j < prods[i].rhs_len; ++j) printf(" %s", symbols[prods[i].rhs[j]]); 
        if (prods[i].rhs_len == 0) printf(" (epsilon)");
        printf("\n"); 
    } 
    
    compute_first(); 
    printf("\nComputed FIRST sets.\n"); 
    
    build_LR1(); 
    printf("Built LR(1) collection with %d states.\n", C_count); 
    
    build_LALR_from_LR1(); 
    printf("Merged to %d LALR states.\n", lalr_count); 
    
    build_parsing_table(); 
    printf("Parsing table built.\n"); 
    
    // optionally print states 
    /*
    for (int i = 0; i < lalr_count; ++i) { 
        printf("\n--- LALR State %d ---\n", i); 
        itemset_print(&lalr_states[i]); 
    } 
    */
  
    // print ACTION table
    printf("\nACTION table:\n"); 
    printf("state\t");
    for (int t = 0; t < sym_count; ++t) if (is_terminal[t]) printf("%s\t", symbols[t]); 
    printf("\n"); 
    for (int s = 0; s < lalr_count; ++s) {
        printf("%d\t", s); 
        for (int t = 0; t < sym_count; ++t) if (is_terminal[t]) { 
            Action a = ACTION[s][t]; 
            if (a.action == 0) printf(" . \t"); 
            else if (a.action == 1) printf("s%d\t", a.number); 
            else if (a.action == 2) printf("r%d\t", a.number); 
            else if (a.action == 3) printf("ACC\t"); 
        } 
        printf("\n"); 
    } 

    // print GOTO table
    printf("\nGOTO table:\n"); 
    printf("state\t");
    for (int t = 0; t < sym_count; ++t) if (!is_terminal[t]) printf("%s\t", symbols[t]); 
    printf("\n"); 
    for (int s = 0; s < lalr_count; ++s) {
        printf("%d\t", s); 
        for (int t = 0; t < sym_count; ++t) if (!is_terminal[t]) { 
            if(GOTO[s][t] == -1) printf(" . \t");
            else printf("%d\t", GOTO[s][t]);
        } 
        printf("\n"); 
    } 
  
    printf("\n--- Parser --- \n");
    while(1) {
        parse_input(); 
        printf("\nParse another? (y/n): ");
        char c = getchar();
        while(getchar() != '\n'); // clear input buffer
        if (c != 'y' && c != 'Y') break;
    }
    return 0; 
}