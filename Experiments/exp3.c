#include <stdio.h> 
#include <ctype.h> 
#include <string.h> 
  
const char *keywords[] = 
{ 
    "auto","break","case","char","const","continue","default","do","double", 
    "else","enum","extern","float","for","goto","if","int","long", 
    "register","return","short","signed","sizeof","static","struct","switch", 
    "typedef","union","unsigned","void","volatile","while" 
}; 
 
int isKeyword(const char *str)  
{ 
    for (int i = 0; i < 32; i++)  
 { 
        if (strcmp(str, keywords[i]) == 0) return 1; 
    } 
    return 0; 
} 
 
int isValidIdentifier(const char *str)  
{ 
    if (!isalpha(str[0]) && str[0] != '_') 
        return 0; 
    for (int i = 1; str[i] != '\0'; i++)  
 { 
        if (!isalnum(str[i]) && str[i] != '_') 
            return 0; 
    } 
    if (isKeyword(str)) 
        return 0; 
    return 1; 
} 
int main()  
{ 
    char str[100]; 
    printf("Enter an identifier: "); 
    scanf("%s", str); 
 
    if (isValidIdentifier(str)) 
        printf("'%s' is a valid identifier.\n", str); 
    else 
        printf("'%s' is not a valid identifier.\n", str); 
 
    return 0; 
} 
 