#include <stdio.h> 
#include <ctype.h> 
#include <string.h> 
#include <stdbool.h> 
 
#define MAX_ID_LENGTH 31 
 
const char *keywords[] =  
 { 
   "auto","double","int","struct","break","else","long", 
      "switch","case","enum","register","typedef","char", 
      "extern","return","union","const","float","short", 
      "unsigned","continue","for","signed","void","default", 
      "goto","sizeof","voltile","do","if","static","while" 
 }; 
 
bool isKeyword(const char *str)  
 { 
     for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) 
  { 
         if (strcmp(str, keywords[i]) == 0) 
            return true; 
  } 
      return false; 
 } 
 
void removeCommentsAndAnalyze(FILE *fp)  
 { 
     char ch, buffer[100]; 
     int i = 0; 
 
     while ((ch = fgetc(fp)) != EOF)  
  { 
         if (isspace(ch)) 
            continue; 
         if (ch == '/')  
   { 
             char next = fgetc(fp); 
             if (next == '/')  
    { 
                 while ((ch = fgetc(fp)) != '\n' && ch != EOF); 
                 continue; 
    } 
     else if (next == '*')  
    { 
                 while (1)  
      { 
                   ch = fgetc(fp); 
                      if (ch == '*')  
        {    
  
                           if ((ch = fgetc(fp)) == '/') 
                              break; 
        } 
                      if (ch == EOF) break; 
      } 
                 continue; 
    } 
   else 