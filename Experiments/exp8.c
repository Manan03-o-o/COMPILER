%{
#include <stdio.h>
int single_comment = 0, multi_comment = 0;
FILE *out;
%}

%%
"//".*              { single_comment++; }    

"/\\*"([^*]|\*+[^*/])*\*+"/"    { multi_comment++; } 

.                   { fputc(yytext[0], out); }
\n                  { fputc('\n', out); }

%%

int main() {
    out = fopen("no_comments.c", "w");
    if (!out) {
        printf("Failed to open output file.\n");
        return 1;
    }
    yylex();
    fclose(out);

    printf("Single-line comments: %d\n", single_comment);
    printf("Multi-line comments: %d\n", multi_comment);
    printf("Cleaned file written to no_comments.c\n");

    return 0;
}
    
            F(); 
            Tprime(); 
        } else { 
            printf("Applying Rule: T' -> e\n"); 
        } 
    }
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
        printf("Matched id '%c'\n", input[i]); 
        i++; 
    } else { 
        error(); 
    } 
}
    printf("Enter an expression: ");
    fgets(input, sizeof(input), stdin);
    E();
    if (input[i] == '\0' || input[i] == '\n') {
        printf("Parsing completed successfully.\n");
    } else {
        printf("Unexpected symbol '%c' at position %d\n", input[i], i);
    }
    return 0;
}