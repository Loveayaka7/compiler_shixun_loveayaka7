#ifndef LEXERGEN
#include <stdio.h>
int main()
{
	int i = 0, input_size = 0, j, mode=0, other_number = 0;
	char input[100],c;
	gets(input);
	while (input[i] != '\n'&& input[i]!='\0')
	{
		i++;
		input_size++;
	}	
	if (input[input_size-1] == '\t')
	{
		input_size--;
	}
	i = 0;
	while(i < input_size)
	{
		c = input[i];
		if (c == '=')
		{
			if(other_number!=0)
			{
			printf("(other,%d)", other_number);
			other_number = 0;
			}
			printf("(relop,=)");
			i++;
			continue;
		}
		else if (c == '<')
		{
			if(other_number!=0)
			{
			printf("(other,%d)", other_number);
			other_number = 0;
			}
			if (i == input_size - 1)
			{
				printf("(relop,<)");
				break;
			}
			if (input[i + 1] == '=')
			{
				printf("(relop,<=)");
				i += 2;
			}
			else if (input[i + 1] == '>')
			{
				printf("(relop,<>)");
				i += 2;
			}
			else
			{
				printf("(relop,<)");
				i++;
			}
			continue;
		}
		else if (c == '>') 
		{
			if(other_number!=0)
			{
			printf("(other,%d)", other_number);
			other_number = 0;
			}
			if (i == input_size - 1)
			{
				printf("(relop,>)");
				break;
			}
			if (input[i + 1] == '=')
			{
				printf("(relop,>=)");
				i += 2;
			}
			else 
			{
				printf("(relop,>)");
				i++;
			}
			continue;
		}
		else
		{
			other_number++;
			i++;
		}
	}	
	if (other_number != 0)
	{
		printf("(other,%d)", other_number);
	}
}
#else
#include "../lex.yy.c"
#endif
