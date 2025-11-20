#include <stdio.h> 
#include <string.h> 
int isOperator(const char *op)  
{ 
    const char *operators[] = { 
        "+", "-", "*", "/", "%", "++", "--", 
        "==", "!=", ">", "<", ">=", "<=", 
        "&&", "||", "!", "=", "+=", "-=", "*=", "/=", "%=" 
    }; 
    int totalOperators = sizeof(operators)/sizeof(operators[0]); 
    for (int i = 0; i < totalOperators; i++) { 
        if (strcmp(op, operators[i]) == 0) 
            return 1; 
    } 
    return 0; 
} 
 
int main() { 
    char input[10]; 
    printf("Enter a symbol: "); 
    scanf("%s", input); 
 
    if (isOperator(input)) 
        printf("'%s' is a valid operator.\n", input); 
    else 
        printf("'%s' is not a valid operator.\n", input); 
    return 0; 
} 