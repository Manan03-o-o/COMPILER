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


