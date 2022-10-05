/*
 * expr.lex : Scanner for a simple
 *            expression parser.
 */

%{
#include "pl0.h"
#include <stdio.h>
int other_number=0;
%}

other ('<'|'>'|'=')
END ('\n'|'\t\n')

%%

"<"        {
            print_other();
            printf("(relop,<)");
           }
"<="       {
            print_other();
            printf("(relop,<=)");
           }
"="        {
            print_other();
            printf("(relop,=)");
           }
"<>"       {
            print_other();
            printf("(relop,<>)");
           }
">"        {
            print_other();
            printf("(relop,>)");
           }
">="       {
            print_other();
            printf("(relop,>=)");
           }

.	   {
            other_number+=1;
           }
{END}	   {
            end();
           }
	    
%%
void getsym()
{
	sym = yylex();
}
void end()
{
	if(other_number!=0)
	{
	 printf("(other,%d)",other_number);
	}
}
void print_other()
{
	if(other_number!=0)
	{
	printf("(other,%d)",other_number);
	other_number=0;
	}
}


























