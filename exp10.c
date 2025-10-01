//construction of LL(1) parsing table
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
void add_symbol(char *,char);
void FIND_FIRST(char *,char);
void FIND_FOLLOW(char *,char);
void FIRST_SHOW();
void FOLLOW_SHOW();
int CREATE_LL1_TABLE();
void PARSING_TABLE_SHOW(int);
void LL1_PARSER(char *);

int top=0;
int t,nt,ch,cr,count;
char FIRST[100][100],FOLLOW[100][100];
char T[100],NT[100],G[100][100],STACK[100];
int LL1[100][100];

void main()
{
int i,j,flag,fl,ch1;
char STR[100];
printf("Enter production rules of grammar in the form A->B\n\n");
flag=1;
fl=1;
while(flag==1)
{
printf("\n1) Insert Production Rules\n2) Show Grammar\n3) Exit");
printf("\nEnter your choice: ");
scanf("%d",&ch1);
switch(ch1)
{
    case 1:printf("Enter number %d rules of grammar: ",cr+1);
scanf("%s",G[cr++]);
for(i=0;i<nt && fl==1;i++)
{
if(NT[i]==G[cr-1][0])
fl=0;
}
if(fl==1)
NT[nt++]=G[cr-1][0];
fl=1;
for(i=3;G[cr-1][i]!='\0';i++)
{
if(!isupper(G[cr-1][i]) && G[cr-1][i]!='!')
{
for(j=0;j<t && fl==1;j++)
{
if(T[j]==G[cr-1][i])
fl=0;
}
if(fl==1)
T[t++]=G[cr-1][i];
fl=1;
}
}
break;

case 2:if(cr>0)
{
printf("\nGrammar");
printf("\nStarting symbol is: %c",G[0][0]);
printf("\nNon-terminal symbol is: ");
for(i=0;i<nt;i++)
printf("%c ",NT[i]);
printf("\nTerminal symbol is: ");
for(i=0;i<t;i++)
printf("%c ",T[i]);
printf("\nProduction rules: ");
for(i=0;i<cr;i++)
printf("%s ",G[i]);
printf("\n");
}
else
{
printf("!enter at least one production rules");
}
break;

case 3:flag=0;
}
}
FIRST_SHOW();
FOLLOW_SHOW();

T[t++]='$';
T[t]='\0';

flag=CREATE_LL1_TABLE();
PARSING_TABLE_SHOW(flag);

if(flag==0){
printf("\nEnter string to be parsed: ");
scanf("%s",STR);
LL1_PARSER(STR);
}
}

void add_symbol(char *s,char c)
{
int i,fl=1;
for(i=0;s[i]!='\0';i++)
{
    if(s[i]==c)
    fl=0;
    }

}
 
void FIRTST_SHOW()
{
    int i,j;

    char arr[100];
    for(i=0;i<nt;i++)
    {
        arr[0]='\0';
       FIND_FIRST(arr,NT[i]);