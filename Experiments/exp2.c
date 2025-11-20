#include <stdio.h> 
#include <string.h> 
 
int main()  
{ 
    char line[1000]; 
    printf("Enter a line: ");
    fgets(line, sizeof(line), stdin); 
 
    
    line[strcspn(line, "\n")] = 0; 
 
    if (strncmp(line, "//", 2) == 0)  
 { 
        printf("It is a single-line comment.\n"); 
    } 
    else if (strncmp(line, "/*", 2) == 0 && strstr(line, "*/") != NULL)  
 { 
        printf("It is a multi-line comment.\n"); 
    } 
    else  
 { 
        printf("It is not a comment.\n"); 
    } 
 
    return 0; 
} 