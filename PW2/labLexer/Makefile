CC       = gcc -g
LEX      = flex -i -I 
INC      = -I../pl0/include 

all: reloplex relop

reloplex:   grammar/relop.lex
	$(LEX)  grammar/relop.lex 
	$(CC)  $(INC) -DLEXERGEN src/labLexer-2.c -o labLexer-2 -ll

relop:   
	$(CC)  $(INC) src/labLexer-2.c -o labLexer-2m -ll

